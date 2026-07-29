#include "tcp_server.hpp"

#include "logger.hpp"

#include <boost/asio.hpp>

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

namespace calculator
{
using boost::asio::ip::tcp;

/// @brief Состояние одной клиентской сессии. Держится живым через shared_ptr,
/// пока есть незавершённые асинхронные операции над сокетом.
struct Session : std::enable_shared_from_this<Session>
{
    tcp::socket m_socket;
    TCPServer::RequestHandler& m_handler;
    std::string m_read_chunk; // временный буфер для одного чтения
    std::string m_accumulator; // накопленные, ещё не разобранные данные

    Session(tcp::socket socket, TCPServer::RequestHandler& handler) :
        m_socket(std::move(socket)), m_handler(handler)
    {
        m_read_chunk.resize(4096);
    }

    void start()
    {
        read();
    }

    void read()
    {
        auto self = shared_from_this();
        m_socket.async_read_some(
            boost::asio::buffer(m_read_chunk),
            [this, self](const boost::system::error_code& ec,
                         std::size_t bytes_transferred) {
                if (ec)
                {
                    Logger::instance().debug("Client disconnected: " +
                                             ec.message());
                    return;
                }

                m_accumulator.append(m_read_chunk.data(), bytes_transferred);
                process_buffer();
                read();
            });
    }

    /// @brief Разбирает все полные сообщения (до '\0') из накопленного буфера.
    void process_buffer()
    {
        std::size_t start = 0;
        std::size_t null_pos = 0;
        while ((null_pos = m_accumulator.find('\0', start)) !=
               std::string::npos)
        {
            std::string request = m_accumulator.substr(start, null_pos - start);
            start = null_pos + 1;
            handle_request(request);
        }
        m_accumulator.erase(0, start);
    }

    void handle_request(const std::string& request)
    {
        Logger::instance().debug("Received request: " + request);
        try
        {
            std::string response = m_handler(request);
            Logger::instance().debug("Sending response: " + response);
            write(response);
        }
        catch (const std::exception& e)
        {
            Logger::instance().error("Handler error: " + std::string(e.what()));
        }
    }

    void write(const std::string& response)
    {
        auto self = shared_from_this();
        auto message = std::make_shared<std::string>(response);
        message->push_back('\0');
        boost::asio::async_write(
            m_socket, boost::asio::buffer(*message),
            [self, message](const boost::system::error_code& ec, std::size_t) {
                if (ec)
                {
                    Logger::instance().error("Send error: " + ec.message());
                }
            });
    }
};

struct TCPServer::Impl
{
    boost::asio::io_context m_io_context;
    tcp::acceptor m_acceptor;
    RequestHandler m_handler;
    std::atomic<bool> m_running{false};
    std::thread m_thread;

    Impl(const std::string& address, int port, RequestHandler handler) :
        m_acceptor(m_io_context,
                   tcp::endpoint(boost::asio::ip::make_address(address),
                                 static_cast<unsigned short>(port))),
        m_handler(std::move(handler))
    {}

    void accept()
    {
        m_acceptor.async_accept([this](const boost::system::error_code& ec,
                                       tcp::socket socket) {
            if (ec)
            {
                if (m_running)
                {
                    Logger::instance().error("Accept error: " + ec.message());
                }
                return;
            }
            Logger::instance().debug("Client connected");
            std::make_shared<Session>(std::move(socket), m_handler)->start();
            accept();
        });
    }
};

TCPServer::TCPServer(const std::string& address, int port,
                     RequestHandler handler) :
    m_impl(std::make_unique<Impl>(address, port, std::move(handler)))
{}

TCPServer::~TCPServer()
{
    stop();
}

void TCPServer::async_run()
{
    m_impl->m_running = true;
    m_impl->accept();
    m_impl->m_thread = std::thread([this] { m_impl->m_io_context.run(); });
}

void TCPServer::stop()
{
    if (!m_impl->m_running.exchange(false))
    {
        return;
    }
    boost::asio::post(m_impl->m_io_context, [this] {
        boost::system::error_code ec;
        m_impl->m_acceptor.close(ec);
    });
    m_impl->m_io_context.stop();
    if (m_impl->m_thread.joinable())
    {
        m_impl->m_thread.join();
    }
    Logger::instance().info("TCP server stopped");
}

} // namespace calculator
