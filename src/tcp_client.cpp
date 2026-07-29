#include "tcp_client.hpp"

#include "logger.hpp"

#include <boost/asio.hpp>

#include <utility>

namespace calculator
{
using boost::asio::ip::tcp;

struct TCPClient::Impl
{
    boost::asio::io_context m_io_context;
    tcp::socket m_socket;
    std::string m_host;
    int m_port;

    Impl(std::string host, int port) :
        m_socket(m_io_context), m_host(std::move(host)), m_port(port)
    {}
};

TCPClient::TCPClient(std::string host, int port) :
    m_impl(std::make_unique<Impl>(std::move(host), port))
{}

TCPClient::~TCPClient()
{
    disconnect();
}

void TCPClient::connect()
{
    tcp::resolver resolver(m_impl->m_io_context);
    auto endpoints =
        resolver.resolve(m_impl->m_host, std::to_string(m_impl->m_port));
    boost::asio::connect(m_impl->m_socket, endpoints);
    Logger::instance().info(
        fmt::format("Connected to {}:{}", m_impl->m_host, m_impl->m_port));
}

std::string TCPClient::send_request(const std::string& request)
{
    std::string message = request;
    message.push_back('\0');
    boost::asio::write(m_impl->m_socket, boost::asio::buffer(message));
    Logger::instance().debug("Sent: " + request);

    std::string response;
    char buffer[4096];
    while (true)
    {
        std::size_t bytes =
            m_impl->m_socket.read_some(boost::asio::buffer(buffer));
        response.append(buffer, bytes);

        auto null_pos = response.find('\0');
        if (null_pos != std::string::npos)
        {
            response.resize(null_pos);
            break;
        }
    }

    Logger::instance().debug("Received: " + response);
    return response;
}

void TCPClient::disconnect()
{
    if (m_impl->m_socket.is_open())
    {
        boost::system::error_code ec;
        m_impl->m_socket.shutdown(tcp::socket::shutdown_both, ec);
        m_impl->m_socket.close(ec);
        Logger::instance().info("Disconnected from server");
    }
}

} // namespace calculator
