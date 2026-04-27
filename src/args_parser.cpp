#include "args_parser.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int count_arguments(const char* str)
{
    if (!str || !*str)
        return 0;

    int count = 0;
    const char* ptr = str;

    // Skip leading whitespace
    while (*ptr && (*ptr == ' ' || *ptr == '\t'))
        ptr++;

    while (*ptr)
    {
        // Found start of an argument
        count++;

        // Skip non-whitespace characters (the argument itself)
        while (*ptr && *ptr != ' ' && *ptr != '\t')
            ptr++;

        // Skip whitespace until next argument or end
        while (*ptr && (*ptr == ' ' || *ptr == '\t'))
            ptr++;
    }

    return count;
}

char** parse_arguments_c(const char* input_str, int* argc_out)
{
    if (!input_str)
    {
        *argc_out = 1;
        char** argv = static_cast<char**>(malloc(1 * sizeof(char*)));
        if (!argv)
            return nullptr;
        argv[0] = const_cast<char*>("calculator");
        return argv;
    }

    int arg_count = count_arguments(input_str);
    *argc_out = arg_count + 1; // +1 for program name

    char** argv = static_cast<char**>(malloc((*argc_out) * sizeof(char*)));
    if (!argv)
        return nullptr;

    // Set program name
    argv[0] = const_cast<char*>("calculator");

    if (arg_count == 0)
    {
        return argv;
    }

    // Make a copy of input string to tokenize
    char* input_copy = strdup(input_str);
    if (!input_copy)
    {
        free(argv);
        return nullptr;
    }

    char* token = strtok(input_copy, " \t");
    int i = 1;

    while (token != nullptr && i < *argc_out)
    {
        argv[i] = strdup(token);
        if (!argv[i])
        {
            // Clean up on error
            for (int j = 1; j < i; j++)
            {
                free(argv[j]);
            }
            free(argv);
            free(input_copy);
            return nullptr;
        }
        token = strtok(nullptr, " \t");
        i++;
    }

    free(input_copy);
    return argv;
}

void free_arguments_c(char** argv, int argc)
{
    if (!argv)
        return;
    for (int i = 1; i < argc; i++)
    { // Skip argv[0] which is const string literal
        if (argv[i])
        {
            free(argv[i]);
        }
    }
    free(argv);
}