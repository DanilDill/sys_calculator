#pragma once
#include <memory>
#include <string>

namespace calculator
{

/// @brief Синхронный TCP клиент для calculator сервиса. Сообщения
/// разделяются нулевым байтом.
class TCPClient
{
public:
    TCPClient(std::string host, int port);
    ~TCPClient();

    TCPClient(const TCPClient&) = delete;
    TCPClient& operator=(const TCPClient&) = delete;

    /// @brief Установить соединение с сервером. Бросает исключение при ошибке.
    void connect();

    /// @brief Отправить запрос и дождаться ответа (блокирующе).
    std::string send_request(const std::string& request);

    /// @brief Закрыть соединение.
    void disconnect();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace calculator
