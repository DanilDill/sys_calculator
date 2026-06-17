#include "dbus_server.hpp"

#include "calculator.hpp"
#include <sys/epoll.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "nlohmann/json.hpp"
#include "logger.hpp"
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
            Logger::instance().debug("request: " + input + "\n");
            auto task = nlohmann::json::parse(input).get<calculator::Task>();
            calculator::Calculator::execute(task);
            nlohmann::json responce = task;
            Logger::instance().debug("responce: " + responce.dump() + "\n");
            return responce.dump();
        }
        catch (const nlohmann::json::exception& e)
        {
            nlohmann::json err;
            err["status"]  = "error";
            err["message"] = e.what();
            Logger::instance().error(e.what());
            return err.dump();
        }
        
    };
    void run()
    {
        m_connection->enterEventLoopAsync();
    }
    Impl()
    {
        m_connection = sdbus::createSystemBusConnection(); // Используем session bus вместо system bus
        m_connection->requestName(SERVICE_NAME);
        m_object = sdbus::createObject(*m_connection, OBJECT_PATH);
        m_object->registerMethod(METHOD_NAME)
            .onInterface(INTERFACE_NAME)
            .implementedAs([this](const std::string& input)
                           { return onCalculate(input); });
        m_object->finishRegistration();
        std::stringstream ss;
        ss << " Service '" << SERVICE_NAME <<  "is running on session bus.\n";
        ss << "Object: " << OBJECT_PATH;
        ss <<"Interface: " << INTERFACE_NAME;
        ss << "Method:    Calculate(string) -> string\n\n";
        ss << "Try in another terminal:\n";
        ss << "  busctl  call " << SERVICE_NAME << " " << OBJECT_PATH <<" " << INTERFACE_NAME<< " Calculate ";
        ss <<  "s '{\"firstValue\": 5, \"operation\": \"+\", \"secondValue\": 3}'\n\n";
        Logger::instance().info(ss.str());
            
        
    };


};


DBusServer::DBusServer()
{
    impl = std::make_unique<Impl>();
}

void DBusServer::run()
{
    if (impl && impl->m_connection)
    {
        impl->run();
    }
    
}

void DBusServer::stop()
{
    if (impl && impl->m_connection) {
        impl->m_connection->leaveEventLoop();
    }
}

DBusServer::~DBusServer()=default;
