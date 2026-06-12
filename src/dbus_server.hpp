#pragma once

#include "task.hpp"

#include <sdbus-c++/sdbus-c++.h>
#include <memory>

struct Impl;
/// @brief server Dbus. input - task in json format, output - result in json format
class DBusServer
{
public:
    /// @brief constructor
    DBusServer();
    ///destructor
    ~DBusServer();
    /// @brief start server;
    void run();
private:
/// @brief implementation
std::unique_ptr<Impl> impl;
};


