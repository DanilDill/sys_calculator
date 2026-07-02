#include "dbus_server.hpp"


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

namespace calculator
{
    struct DBusServer::Impl
{
    std::unique_ptr<sdbus::IConnection> m_connection;
    std::unique_ptr<sdbus::IObject> m_object;
    void run()
    {
        m_connection->enterEventLoop();
    }
    void async_run()
    {
        m_connection->enterEventLoopAsync();
    }
    Impl(std::function<std::string(const std::string&)>&& handler)
    {
        m_connection = sdbus::createSystemBusConnection(); // Используем session bus вместо system bus
        m_connection->requestName(SERVICE_NAME);
        m_object = sdbus::createObject(*m_connection, OBJECT_PATH);
        m_object->registerMethod(METHOD_NAME)
            .onInterface(INTERFACE_NAME)
            .implementedAs(handler);
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


DBusServer::DBusServer(std::function<std::string(const std::string&)>&& handler)
{
    m_impl = std::make_unique<Impl>(std::forward<std::function<std::string(const std::string&)>>(handler));
}

void DBusServer::run()
{
    if (m_impl && m_impl->m_connection)
    {
        m_impl->run();
    }
    
}

void DBusServer::async_run()
{
    if (m_impl && m_impl->m_connection)
    {
        m_impl->async_run();
    }
}
void DBusServer::stop()
{
    if (m_impl && m_impl->m_connection) {
        m_impl->m_connection->leaveEventLoop();
    }
}

DBusServer::~DBusServer()=default;

}