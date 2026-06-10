#pragma once

#include "task.hpp"

#include <sdbus-c++/sdbus-c++.h>
#include <memory>

struct Impl;
class DBusServer
{
public:
    DBusServer();
    ~DBusServer();
    void run();
private:
std::unique_ptr<Impl> impl;
};


