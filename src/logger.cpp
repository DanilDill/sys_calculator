#include "logger.hpp"
#include <spdlog/spdlog.h>
Logger::Logger()
{
}
void Logger::init(bool is_debug)
{
    spdlog::set_level(is_debug ? spdlog::level::debug : spdlog::level::info);
}
Logger::~Logger(){}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::debug(std::string_view message)
{
    spdlog::debug(message);
}
void Logger::info(std::string_view message)
{
    spdlog::info(message);
}
void Logger::warning(std::string_view message)
{
    spdlog::warn(message);
}
void Logger::error(std::string_view message)
{
    spdlog::error(message);
}
