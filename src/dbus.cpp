#include "dbus.hpp"

#include "calculator.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
static dbus_bool_t handle_method_call(DBusConnection* conn, DBusMessage* msg)
{
    if (!dbus_message_is_method_call(msg, INTERFACE_NAME, METHOD_NAME))
    {
        return FALSE;
    }

    // Проверяем путь объекта
    if (strcmp(dbus_message_get_path(msg), OBJECT_PATH) != 0)
    {
        return FALSE;
    }

    const char* input = nullptr;
    DBusError err;
    dbus_error_init(&err);

    if (!dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &input,
                               DBUS_TYPE_INVALID))
    {
        fprintf(stderr, "Failed to get argument: %s\n", err.message);
        dbus_error_free(&err);

        DBusMessage* reply = dbus_message_new_error(
            msg, DBUS_ERROR_INVALID_ARGS, "Expected a string argument");
        if (reply)
        {
            dbus_connection_send(conn, reply, nullptr);
            dbus_message_unref(reply);
        }
        return TRUE; // ошибка обработана
    }

    if (input == nullptr)
    {
        input = "0"; // Default to 0 if no input
    }

    int errcode = 0;
    calculator::Task* task = calculator::make_task(input, errcode);
    char* result;
    if (errcode != 0 || result == nullptr)
    {
        result = nullptr;
    }
    else
    {
        calculator::run(task);
        result = output(task);
    }
    DBusMessage* reply = dbus_message_new_method_return(msg);
    if (!reply)
    {
        if (result)
        {
            free(result); // Free calculator result if reply creation failed
        }
        fprintf(stderr, "Failed to create reply message\n");
        return FALSE;
    }

    const char* response_str =
        result ? result : calculator::error_to_string(errcode);

    if (!dbus_message_append_args(reply, DBUS_TYPE_STRING, &response_str,
                                  DBUS_TYPE_INVALID))
    {
        fprintf(stderr, "Failed to append reply args\n");
        if (result)
        {
            free(result);
        }
        dbus_message_unref(reply);
        return FALSE;
    }

    if (!dbus_connection_send(conn, reply, nullptr))
    {
        fprintf(stderr, "Failed to send reply\n");
        if (result)
        {
            free(result);
        }
        dbus_message_unref(reply);

        return FALSE;
    }

    // Free calculator result after successful send
    if (result)
    {
        free(result);
    }

    dbus_connection_flush(conn);
    dbus_message_unref(reply);
    return TRUE;
}
namespace dbus
{
DBusConnection* init()
{
    DBusError err;
    dbus_error_init(&err);

    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Cannot connect to session bus: %s\n", err.message);
        dbus_error_free(&err);
        return nullptr;
    }

    if (!conn)
    {
        fprintf(stderr, "Failed to get D-Bus connection\n");
        return nullptr;
    }

    int ret = dbus_bus_request_name(conn, SERVICE_NAME,
                                    DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);
    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Cannot acquire service name: %s\n", err.message);
        dbus_error_free(&err);
        dbus_connection_unref(conn);
        return nullptr;
    }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
        fprintf(stderr, "Name already taken or not primary owner (code %d)\n",
                ret);
        dbus_connection_unref(conn);
        return nullptr;
    }

    printf("✅ Service '%s' is running on session bus.\n", SERVICE_NAME);
    printf("Object: %s\n", OBJECT_PATH);
    printf("Interface: %s\n", INTERFACE_NAME);
    printf("Method: Calculate(string) -> string\n");
    printf("\nTry in another terminal:\n");
    printf("  busctl  call %s %s %s %s  s \"5 + 5\"\n\n", SERVICE_NAME,
           OBJECT_PATH, INTERFACE_NAME, METHOD_NAME);
    return conn;
}
void start_event_loop(DBusConnection* conn)
{
    while (true)
    {
        // Ждём до 100 мс новых сообщений
        dbus_connection_read_write_dispatch(conn, 100);

        DBusMessage* msg = dbus_connection_pop_message(conn);
        while (msg != nullptr)
        {
            if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_CALL)
            {
                if (!handle_method_call(conn, msg))
                {
                    // Неизвестный метод — отправим ошибку
                    DBusMessage* error_reply = dbus_message_new_error(
                        msg, DBUS_ERROR_UNKNOWN_METHOD,
                        "Method not found or wrong object path");
                    if (error_reply)
                    {
                        dbus_connection_send(conn, error_reply, NULL);
                        dbus_message_unref(error_reply);
                    }
                }
            }
            dbus_message_unref(msg);
            msg = dbus_connection_pop_message(conn);
        }
    }
}
void deinit(DBusConnection* conn)
{
    dbus_connection_close(conn);
    dbus_connection_unref(conn);
}
} // namespace dbus
