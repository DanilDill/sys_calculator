#include "dbus_server.hpp"
#include <memory>
class app
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
public:
    app(int argc, char** argv);
    ~app();
    void run();
    void stop();
    
private:
    bool parce_cli_args(int argc, char** argv);
    void print_help(std::string_view app_name);
};

