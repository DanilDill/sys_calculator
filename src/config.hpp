#pragma once
#include <nlohmann/json.hpp>

#include <string>

namespace calculator
{

/// @brief Режим работы сервиса: только D-Bus, только TCP или оба сразу
enum class ServiceMode
{
    DBUS_ONLY,
    TCP_ONLY,
    BOTH
};

/// @brief Конфигурация сервиса, загружаемая из JSON файла
struct Config
{
    ServiceMode mode{ServiceMode::DBUS_ONLY};
    std::string tcp_address{"0.0.0.0"};
    int tcp_port{1234};
    std::string log_level{"info"};
    std::string postgres_connection{
        "host=localhost dbname=calc user=calc password=calc_password"};
    std::string redis_uri{"tcp://127.0.0.1:6379"};

    /// @brief Загрузить конфиг из файла. Бросает std::runtime_error при
    /// невозможности прочитать/распарсить файл
    static Config load_from_file(const std::string& path);

    /// @brief Разобрать конфиг из уже распарсенного JSON. Отсутствующие поля
    /// остаются со значениями по умолчанию
    static Config parse_json(const nlohmann::json& json);
};

} // namespace calculator
