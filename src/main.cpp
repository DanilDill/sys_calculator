// #include <dbus-1.0/dbus/dbus.h>
// #include "app.h"
// int main(int argc, char** argv)
// {
//   app::run(argc, argv);
// }
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVICE_NAME      "com.example.HelloService"
#define OBJECT_PATH       "/com/example/HelloObject"
#define INTERFACE_NAME    "com.example.HelloInterface"

static dbus_bool_t handle_method_call(DBusConnection *conn, DBusMessage *msg) {
    if (!dbus_message_is_method_call(msg, INTERFACE_NAME, "SayHello")) {
        return FALSE;
    }

    // Проверяем путь объекта
    if (strcmp(dbus_message_get_path(msg), OBJECT_PATH) != 0) {
        return FALSE;
    }

    const char *input_name = NULL;
    DBusError err;
    dbus_error_init(&err);

    if (!dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &input_name, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "Failed to get argument: %s\n", err.message);
        dbus_error_free(&err);

        DBusMessage *reply = dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS, "Expected a string argument");
        if (reply) {
            dbus_connection_send(conn, reply, NULL);
            dbus_message_unref(reply);
        }
        return TRUE; // ошибка обработана
    }

    if (input_name == NULL) {
        input_name = "anonymous";
    }

    char greeting[512];
    snprintf(greeting, sizeof(greeting), "Hello, %s!", input_name);

    DBusMessage *reply = dbus_message_new_method_return(msg);
    if (!reply) {
        fprintf(stderr, "Failed to create reply message\n");
        return FALSE;
    }

    const char *greeting_cstr = greeting;
    if (!dbus_message_append_args(reply, DBUS_TYPE_STRING, &greeting_cstr, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "Failed to append reply args\n");
        dbus_message_unref(reply);
        return FALSE;
    }

    if (!dbus_connection_send(conn, reply, NULL)) {
        fprintf(stderr, "Failed to send reply\n");
        dbus_message_unref(reply);
        return FALSE;
    }

    dbus_connection_flush(conn);
    dbus_message_unref(reply);
    return TRUE;
}

/*
busctl --user call com.example.HelloService \
                   /com/example/HelloObject \
                   com.example.HelloInterface \
                   SayHello \
                   s "Bob"
*/
int main(void) {
    DBusError err;
    dbus_error_init(&err);

    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Cannot connect to session bus: %s\n", err.message);
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    if (!conn) {
        fprintf(stderr, "Failed to get D-Bus connection\n");
        return EXIT_FAILURE;
    }

    int ret = dbus_bus_request_name(conn, SERVICE_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Cannot acquire service name: %s\n", err.message);
        dbus_error_free(&err);
        dbus_connection_unref(conn);
        return EXIT_FAILURE;
    }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf(stderr, "Name already taken or not primary owner (code %d)\n", ret);
        dbus_connection_unref(conn);
        return EXIT_FAILURE;
    }

    printf("✅ Service '%s' is running on session bus.\n", SERVICE_NAME);
    printf("Object: %s\n", OBJECT_PATH);
    printf("Interface: %s\n", INTERFACE_NAME);
    printf("Method: SayHello(string) -> string\n");
    printf("\nTry in another terminal:\n");
    printf("  busctl --user call %s %s %s SayHello s \"Alice\"\n\n", 
           SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME);

    while (1) {
        // Ждём до 100 мс новых сообщений
        dbus_connection_read_write_dispatch(conn, 100);

        DBusMessage *msg = dbus_connection_pop_message(conn);
        while (msg != NULL) {
            if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
                if (!handle_method_call(conn, msg)) {
                    // Неизвестный метод — отправим ошибку
                    DBusMessage *error_reply = dbus_message_new_error(
                        msg, DBUS_ERROR_UNKNOWN_METHOD, "Method not found or wrong object path");
                    if (error_reply) {
                        dbus_connection_send(conn, error_reply, NULL);
                        dbus_message_unref(error_reply);
                    }
                }
            }
            dbus_message_unref(msg);
            msg = dbus_connection_pop_message(conn);
        }
    }

    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}