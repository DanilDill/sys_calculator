
#include "dbus.hpp"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
Example usage:
busctl -- call com.example.CalculatorService \
                   /com/example/CalculatorObject \
                   com.example.CalculatorInterface \
                   Calculate \
                   s "1 + 4"
*/

void print_help(const char* program_name)
{
    printf("Usage: %s\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message and exit\n");
    printf("\n");
    printf("Run: %s\n", program_name);
    printf("In other terminal run:\n");
    printf("  busctl call com.example.CalculatorService \\\n");
    printf("             /com/example/CalculatorObject \\\n");
    printf("             com.example.CalculatorInterface \\\n");
    printf("             Calculate \\\n");
    printf("             s \"1 + 4\"\n");
}

bool parce_cli_args(int argc, char** argv)
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

int application_run(int argc, char** argv)
{
    if (!parce_cli_args(argc, argv))
    {
        return EXIT_FAILURE;
    }
    auto* conn = dbus::init();
    if (conn == nullptr)
    {
        return EXIT_FAILURE;
    }
    dbus::start_event_loop(conn);
    dbus::deinit(conn);

    return EXIT_SUCCESS;
}
int main(int argc, char** argv)
{
    return application_run(argc, argv);
}