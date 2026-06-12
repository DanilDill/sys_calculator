#include "app.hpp"
#include <getopt.h>
#include <iostream>
app::app(int argc, char** argv)
{
    m_is_run = parce_cli_args(argc, argv);
}

bool app::parce_cli_args(int argc, char** argv)
{
    int opt;
    int option_index = 0;
    static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                           {0, 0, 0, 0}};
    bool is_run = true;
    while ((opt = getopt_long(argc, argv, "h", long_options, &option_index)) !=
           -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(argv[0]);
                is_run = false;
                break;
            case '?':
                fprintf(stderr, "Try '%s --help' for more information.\n",
                        argv[0]);
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
    std::cout << "Usage: "<< program_name << std::endl;
    std::cout << std::endl;
    std::cout << "Options:"<<std::endl;
    std::cout << "  -h, --help     Show this help message and exit" << std::endl;
    std::cout << "\n";
    std::cout << "Run: "<< program_name << std::endl;
    std::cout << "In other terminal run:\n";
    std::cout << "  busctl call com.example.CalculatorService \\\n";
    std::cout << "             /com/example/CalculatorObject \\\n";
    std::cout << "             com.example.CalculatorInterface \\\n";
    std::cout << "             Calculate \\\n";
    std::cout << "              s '{\"firstValue\": 5, \"operation\": \"+\", \"secondValue\": 3}'\n\n";
}

void app::run()
{
    if (m_is_run)
    {
        return;
    }
    
    try
    {
        DBusServer server;
        server.run();
    }
    catch (const sdbus::Error& e)
    {
        std::cerr << "D-Bus error: " << e.getName()
                  << " — " << e.getMessage() << "\n";
        return;
    }
    return;    
}

void app::stop()
{

}