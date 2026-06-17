#include "app.hpp"
#include <getopt.h>
#include <iostream>
#include "logger.hpp"
#include <sstream>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <signal.h>
app::app(int argc, char** argv)
{
    m_is_run = parce_cli_args(argc, argv);
}

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
                m_is_debug = true;
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

int setupSignalFd()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    // Сигналы уже заблокированы в run(); signalfd "подписывается" на них.
    int sfd = signalfd(-1, &mask, SFD_CLOEXEC);
    if (sfd == -1) {
        Logger::instance().error(std::string("signalfd failed: ") +
                                 std::strerror(errno));
    }
    return sfd;
}

void waitForShutdown(int sfd)
{
    int epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd == -1) {
        Logger::instance().error(std::string("epoll_create1 failed: ") +
                                 std::strerror(errno));
        return;
    }

    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = sfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1) {
        Logger::instance().error("epoll_ctl failed");
        ::close(epfd);
        return;
    }

    epoll_event events[4];
    bool running = true;

    while (running) {
        int n = epoll_wait(epfd, events, 4, -1);

        if (n == -1) {
            if (errno == EINTR) {
                continue; 
            }
            Logger::instance().error(std::string("epoll_wait failed: ") +
                                     std::strerror(errno));
            break;
        }

        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == sfd) {
                signalfd_siginfo si;
                ssize_t r = ::read(sfd, &si, sizeof(si));
                if (r != static_cast<ssize_t>(sizeof(si))) {
                    continue;
                }

                // ВНЕ сигнального контекста — логгер безопасен!
                Logger::instance().info("Received signal " +
                                        std::to_string(si.ssi_signo) +
                                        ", initiating shutdown");
                running = false;
            }
        }
    }

    ::close(epfd);
}


void app::run()
{
    if (!m_is_run)
    {
        return;
    }
    Logger::init(m_is_debug);
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    if (pthread_sigmask(SIG_BLOCK, &mask, nullptr) != 0) {
        Logger::instance().error("pthread_sigmask failed");
        return;
    }


    try
    {
        m_server = std::make_unique<DBusServer>();
        m_server->run();
    }
    catch (const sdbus::Error& e)
    {
        std::stringstream ss;
        ss << "D-Bus error: " << e.getName()
                  << " — " << e.getMessage() << "\n";
        Logger::instance().error(ss.str());
        return;
    }
Logger::instance().info("Server started, waiting for signals");

    // 3) Создаём signalfd и ждём сигнала через epoll
    int sfd = setupSignalFd();
    if (sfd != -1) {
        waitForShutdown(sfd);   // блокируется до SIGINT/SIGTERM
        ::close(sfd);
    }

    // 4) Штатная остановка — leaveEventLoop, деструкторы отработают
    Logger::instance().info("Stopping server");
    stop();


    return;    
}

void app::stop()
{
    if (m_server) {
        m_server->stop();
    }
}