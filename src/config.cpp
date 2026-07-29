#include "config.hpp"

#include <fmt/format.h>

#include <fstream>
#include <stdexcept>

namespace calculator
{
namespace
{
ServiceMode parse_mode(const std::string& mode)
{
    if (mode == "dbus" || mode == "dbus_only")
    {
        return ServiceMode::DBUS_ONLY;
    }
    if (mode == "tcp" || mode == "tcp_only")
    {
        return ServiceMode::TCP_ONLY;
    }
    if (mode == "both")
    {
        return ServiceMode::BOTH;
    }
    throw std::runtime_error(fmt::format(
        "invalid 'mode' value: '{}' (expected dbus_only|tcp_only|both)", mode));
}
} // namespace

Config Config::parse_json(const nlohmann::json& json)
{
    Config cfg;

    if (json.contains("mode"))
    {
        cfg.mode = parse_mode(json.at("mode").get<std::string>());
    }
    if (json.contains("tcp_address"))
    {
        cfg.tcp_address = json.at("tcp_address").get<std::string>();
    }
    if (json.contains("tcp_port"))
    {
        cfg.tcp_port = json.at("tcp_port").get<int>();
        if (cfg.tcp_port <= 0 || cfg.tcp_port > 65535)
        {
            throw std::runtime_error(fmt::format(
                "invalid 'tcp_port': {} (expected 1..65535)", cfg.tcp_port));
        }
    }
    if (json.contains("log_level"))
    {
        cfg.log_level = json.at("log_level").get<std::string>();
    }
    if (json.contains("postgres_connection"))
    {
        cfg.postgres_connection =
            json.at("postgres_connection").get<std::string>();
    }
    if (json.contains("redis_uri"))
    {
        cfg.redis_uri = json.at("redis_uri").get<std::string>();
    }

    return cfg;
}

Config Config::load_from_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error(
            fmt::format("cannot open config file: {}", path));
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(file);
        return parse_json(json);
    }
    catch (const nlohmann::json::exception& e)
    {
        throw std::runtime_error(
            fmt::format("failed to parse config file {}: {}", path, e.what()));
    }
}

} // namespace calculator
