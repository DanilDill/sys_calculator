#pragma once

#include "task.hpp"

#include <sdbus-c++/sdbus-c++.h>
#include <memory>
#include <functional>
namespace calculator
{
    /// @brief server Dbus. input - task in json format, output - result in json format
class DBusServer
{
public:
    /// @brief constructor
    DBusServer(std::function<std::string(const std::string&)>&&);
    ///destructor
    ~DBusServer();
    /// @brief start server;
    void run();
    /// @brief start server asyncronously; 
    void async_run();
    /// @brief stop server gracefully
    void stop();
private:
/// @brief implementation
struct Impl;
std::unique_ptr<Impl> m_impl;
};

}
