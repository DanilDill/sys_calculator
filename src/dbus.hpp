#pragma once

#include "task.hpp"

extern "C" {
    #include <dbus/dbus.h>
}

constexpr char* SERVICE_NAME = "com.example.CalculatorService";
constexpr char* OBJECT_PATH = "/com/example/CalculatorObject";
constexpr char* INTERFACE_NAME = "com.example.CalculatorInterface";
constexpr char* METHOD_NAME = "Calculate";

namespace dbus
{
DBusConnection* init();
void start_event_loop(DBusConnection* conn);
void deinit(DBusConnection* conn);
} // namespace dbus
