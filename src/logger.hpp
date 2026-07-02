#pragma once
#include "string_view"
#include <fmt/format.h>
class Logger
{
    public:
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;
    

    void debug(std::string_view message);
    void info(std::string_view message);
    void warning(std::string_view message);
    void error(std::string_view message);
    static Logger& instance();
    static void init(bool is_debug);
    private:
    Logger();
    ~Logger();
};