#include "config.hpp"
#include "dbus_server.hpp"
#include "tcp_server.hpp"

#include <memory>
namespace calculator
{
class app
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
public:
    app(int argc, char** argv);
    ~app();
    app(app&&) = delete;
    app(const app&) = delete;
    app& operator=(const app&) = delete;
    app& operator=(app&& other) noexcept = delete;
    void run();
    void stop();

private:
    bool parse_cli_args(int argc, char** argv);
    void print_help(std::string_view app_name);
    void load_config();
};

}
