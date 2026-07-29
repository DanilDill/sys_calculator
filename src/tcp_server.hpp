#pragma once
#include <functional>
#include <memory>
#include <string>

namespace calculator
{

/// @brief TCP сервер. Принимает JSON запросы, разделённые нулевым байтом,
/// передаёт их обработчику и отправляет обратно ответ, также завершённый '\0'.
class TCPServer
{
public:
    using RequestHandler = std::function<std::string(const std::string&)>;

    /// @brief Создаёт сервер, слушающий address:port. Bind/listen происходит
    /// в конструкторе — при ошибке бросается исключение.
    TCPServer(const std::string& address, int port, RequestHandler handler);
    ~TCPServer();

    TCPServer(const TCPServer&) = delete;
    TCPServer& operator=(const TCPServer&) = delete;

    /// @brief Запустить event loop в отдельном потоке.
    void async_run();

    /// @brief Корректно остановить сервер и дождаться завершения потока.
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace calculator
