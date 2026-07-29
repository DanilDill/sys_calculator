#include "app.hpp"

#include "calculator.hpp"
#include "history_service.hpp"
#include "logger.hpp"

#include <getopt.h>
#include <signal.h>

#include <iostream>
#include <sstream>
namespace calculator
{
struct app::Impl
{
    std::unique_ptr<DBusServer> m_dbus_server{nullptr};
    std::unique_ptr<TCPServer> m_tcp_server{nullptr};
    bool m_is_run{false};
    Config m_config;
    std::string m_config_path{"/usr/local/etc/calculator.json"};
};
app::app(int argc, char** argv)
{
    m_impl = std::make_unique<Impl>();
    m_impl->m_is_run = parse_cli_args(argc, argv);
}
app::~app() = default;
bool app::parse_cli_args(int argc, char** argv)
{
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"config", required_argument, 0, 'c'},
        {0, 0, 0, 0}};
    bool is_run = true;
    while ((opt = getopt_long(argc, argv, "hc:", long_options,
                              &option_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(argv[0]);
                is_run = false;
                break;
            case 'c':
                m_impl->m_config_path = optarg;
                break;
            case '?':
                Logger::instance().error(fmt::format(
                    "Try '{} --help' for more information.", argv[0]));
                is_run = false;
                break;
            default:
                break;
        }
    }
    return is_run;
}

void app::print_help(std::string_view program_name)
{
    std::stringstream ss;
    ss << "Usage: " << program_name << " [-c CONFIG]" << std::endl;
    ss << std::endl;
    ss << "Options:" << std::endl;
    ss << "  -h, --help          Show this help message and exit" << std::endl;
    ss << "  -c, --config PATH   Path to JSON config file "
          "(default: /usr/local/etc/calculator.json)"
       << std::endl;
    ss << "\n";
    ss << "Run: " << program_name << " --config /usr/local/etc/calculator.json"
       << std::endl;
    Logger::instance().info(ss.str());
}

void app::load_config()
{
    m_impl->m_config = Config::load_from_file(m_impl->m_config_path);
    Logger::instance().info(
        fmt::format("Config loaded from {}", m_impl->m_config_path));
}

void app::run()
{
    if (!m_impl->m_is_run)
    {
        return;
    }
    try
    {
        load_config();
    }
    catch (const std::exception& e)
    {
        Logger::instance().error(
            fmt::format("Failed to load config: {}", e.what()));
        return;
    }
    Logger::init(m_impl->m_config.log_level == "debug");
    try
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGINT);
        sigaddset(&set, SIGTERM);

        if (int mask = pthread_sigmask(SIG_BLOCK, &set, nullptr); mask != 0)
        {
            perror("pthread_sigmask");
            throw std::runtime_error(
                fmt::format("pthread_sigmask return: {}", mask));
        }
        auto history = std::make_shared<HistoryService>(
            std::make_unique<storage::PostgresStorage>(
                m_impl->m_config.postgres_connection),
            std::make_unique<storage::RedisCache>(m_impl->m_config.redis_uri));

        auto onCalculate = [history](const std::string& input) {
            try
            {
                Logger::instance().debug("request: " + input + "\n");
                auto task =
                    nlohmann::json::parse(input).get<calculator::Task>();
                history->process(task);
                nlohmann::json responce = task;
                Logger::instance().debug("responce: " + responce.dump() + "\n");
                return responce.dump();
            }
            catch (const nlohmann::json::exception& e)
            {
                nlohmann::json err;
                err["status"] = "error";
                err["message"] = e.what();
                Logger::instance().error(e.what());
                return err.dump();
            }
        };

        switch (m_impl->m_config.mode)
        {
            case ServiceMode::DBUS_ONLY:
                m_impl->m_dbus_server = std::make_unique<DBusServer>(
                    std::function<std::string(const std::string&)>(
                        onCalculate));
                m_impl->m_dbus_server->async_run();
                Logger::instance().info("D-Bus server started");
                break;

            case ServiceMode::TCP_ONLY:
                m_impl->m_tcp_server = std::make_unique<TCPServer>(
                    m_impl->m_config.tcp_address, m_impl->m_config.tcp_port,
                    onCalculate);
                m_impl->m_tcp_server->async_run();
                Logger::instance().info(fmt::format(
                    "TCP server started on {}:{}", m_impl->m_config.tcp_address,
                    m_impl->m_config.tcp_port));
                break;

            case ServiceMode::BOTH:
                m_impl->m_dbus_server = std::make_unique<DBusServer>(
                    std::function<std::string(const std::string&)>(
                        onCalculate));
                m_impl->m_dbus_server->async_run();
                m_impl->m_tcp_server = std::make_unique<TCPServer>(
                    m_impl->m_config.tcp_address, m_impl->m_config.tcp_port,
                    onCalculate);
                m_impl->m_tcp_server->async_run();
                Logger::instance().info(fmt::format(
                    "Both D-Bus and TCP servers started (TCP on {}:{})",
                    m_impl->m_config.tcp_address, m_impl->m_config.tcp_port));
                break;
        }

        Logger::instance().info("Server started, waiting for signals");

        int sig;
        sigwait(&set, &sig);
        Logger::instance().info(
            fmt::format("Signal received: {}. Stopping servers...", sig));

        if (m_impl->m_dbus_server)
        {
            m_impl->m_dbus_server->stop();
        }
        if (m_impl->m_tcp_server)
        {
            m_impl->m_tcp_server->stop();
        }
        Logger::instance().info("Servers stopped");
    }
    catch (const sdbus::Error& e)
    {
        std::stringstream ss;
        ss << "D-Bus error: " << e.getName() << " — " << e.getMessage() << "\n";
        Logger::instance().error(ss.str());
        return;
    }
    catch (const std::exception& e)
    {
        // fail-fast: без Postgres на старте не поднимаемся, systemd
        // перезапустит
        Logger::instance().error(
            fmt::format("fatal error on startup: {}", e.what()));
        return;
    }
}

void app::stop()
{
    if (m_impl->m_dbus_server)
    {
        m_impl->m_dbus_server->stop();
    }
    if (m_impl->m_tcp_server)
    {
        m_impl->m_tcp_server->stop();
    }
}
} // namespace calculator
