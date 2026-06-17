#include "dbus_server.hpp"
#include <memory>
class app
{
private:
    std::unique_ptr<DBusServer> m_server{nullptr};
    bool m_is_run{false};
    bool m_is_debug{false};
public:
    app(int argc, char** argv);
    ~app()=default;
    void run();
    void stop();
    
private:
    bool parce_cli_args(int argc, char** argv);
    void print_help(std::string_view app_name);
};

