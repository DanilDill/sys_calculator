#include "app.hpp"
#include <getopt.h>
#include <iostream>
#include "logger.hpp"
#include <sstream>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <signal.h>

struct app::Impl
{
    std::unique_ptr<DBusServer> m_server{nullptr};
    bool m_is_run{false};
    bool m_is_debug{false};
    inline static int m_pipe[2]{};
    static std::unordered_map<int, std::function<void(int)>> msignal_handlers;
    static void onSignal(int signum)
    {
        unsigned char byte = static_cast<unsigned char>(signum);
        ssize_t r = write(m_pipe[1], &byte, 1);
    }
};
app::app(int argc, char** argv)
{
    m_impl = std::make_unique<Impl>();
    m_impl->m_is_run = parce_cli_args(argc, argv);
}
app::~app() = default;
bool app::parce_cli_args(int argc, char** argv)
{
    int opt;
    int option_index = 0;
    static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                           {"debug", no_argument, 0, 'd'},
                                           {0, 0, 0, 0}};
    bool is_run = true;
    while ((opt = getopt_long(argc, argv, "hd", long_options, &option_index)) !=
           -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(argv[0]);
                is_run = false;
                break;
            case '?':
                Logger::instance().error(fmt::format("Try '{} --help' for more information.",argv[0]));
                is_run = false;
                break;
            case 'd':
                m_impl->m_is_debug = true;
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
    ss << "Usage: "<< program_name << std::endl;
    ss << std::endl;
    ss << "Options:"<<std::endl;
    ss << "  -h, --help     Show this help message and exit" << std::endl;
    ss << "\n";
    ss << "Run: "<< program_name << std::endl;
    ss << "In other terminal run:\n";
    ss << "  busctl call com.example.CalculatorService \\\n";
    ss << "             /com/example/CalculatorObject \\\n";
    ss << "             com.example.CalculatorInterface \\\n";
    ss << "             Calculate \\\n";
    ss << "              s '{\"firstValue\": 5, \"operation\": \"+\", \"secondValue\": 3}'\n\n";
    Logger::instance().info(ss.str());
}


void app::run()
{
    if (!m_impl->m_is_run)
    {
        return;
    }
    Logger::init(m_impl->m_is_debug);
    try
    {
        if (pipe(app::Impl::m_pipe) == -1) 
        { 
            Logger::instance().error("Pipe error");
            return; 
        }
        
        static struct sigaction sa{};
        sa.sa_handler = Impl::onSignal;
        sa.sa_flags = SA_RESTART;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGINT,  &sa, nullptr);

        m_impl->m_server = std::make_unique<DBusServer>();
        m_impl->m_server->async_run();
        Logger::instance().info("Server started, waiting for signals");
        
        unsigned char buf;
        ssize_t n = read(Impl::m_pipe[0], &buf, 1);
        Logger::instance().info(fmt::format("Signal received: {}. stop Dbus",static_cast<int>(buf)));
        m_impl->m_server->stop();
        close(Impl::m_pipe[0]);
        close(Impl::m_pipe[1]); 
        Logger::instance().info("Stopping server");

    }
    catch (const sdbus::Error& e)
    {
        std::stringstream ss;
        ss << "D-Bus error: " << e.getName()
                  << " — " << e.getMessage() << "\n";
        Logger::instance().error(ss.str());
        return;
    }

}

void app::stop()
{
    if (m_impl->m_server) {
        m_impl->m_server->stop();
    }
}