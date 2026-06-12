#include "dbus_server.hpp"

#include "calculator.hpp"
#include <sys/epoll.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "nlohmann/json.hpp"
namespace
{
constexpr const char* SERVICE_NAME = "com.example.CalculatorService";
constexpr const char* OBJECT_PATH = "/com/example/CalculatorObject";
constexpr const char* INTERFACE_NAME = "com.example.CalculatorInterface";
constexpr const char* METHOD_NAME = "Calculate";
};

struct Impl
{
    std::unique_ptr<sdbus::IConnection> m_connection;
    std::unique_ptr<sdbus::IObject> m_object;
    std::string onCalculate(const std::string& input)
    {
        try
        {
            auto task = nlohmann::json::parse(input).get<calculator::Task>();
            calculator::Calculator::execute(task);
            nlohmann::json responce = task;
            return responce.dump();
        }
        catch (const nlohmann::json::exception& e)
        {
            nlohmann::json err;
            err["status"]  = "error";
            err["message"] = e.what();
            return err.dump();
        }
        
    };
    void run()
    {
        m_connection->enterEventLoop();
    }
    Impl()
    {
        m_connection = sdbus::createSessionBusConnection();//sdbus::createSystemBusConnection();
        m_connection->requestName(SERVICE_NAME);
        m_object = sdbus::createObject(*m_connection, OBJECT_PATH);
        m_object->registerMethod(METHOD_NAME)
            .onInterface(INTERFACE_NAME)
            .implementedAs([this](const std::string& input)
                           { return onCalculate(input); });
        m_object->finishRegistration();
        std::printf("✅ Service '%s' is running on system bus.\n", SERVICE_NAME);
        std::printf("Object:    %s\n", OBJECT_PATH);
        std::printf("Interface: %s\n", INTERFACE_NAME);
        std::printf("Method:    Calculate(string) -> string\n\n");
        std::printf("Try in another terminal:\n");
        std::printf("  busctl --user call %s %s %s Calculate "
            "s '{\"firstValue\": 5, \"operation\": \"+\", \"secondValue\": 3}'\n\n",
            SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME);
        
    };


};


DBusServer::DBusServer()
{
    impl = std::make_unique<Impl>();
}

void DBusServer::run()
{
    impl->run();
}

DBusServer::~DBusServer()=default;
