
#include "task.hpp"

#include "args_parser.hpp"

#include <cstdio>
#include <cstdlib>

namespace calculator
{
int parse(int argc, char** argv, Task& task)
{
    if (argc == 4)
    {
        task.value1 = atoi(argv[1]);
        task.operation = *(argv[2]);
        task.value2 = atoi(argv[3]);
        return 0;
    }
    else if (argc == 2)
    {
        char* endptr;
        task.value1 = strtol(argv[1], &endptr, 10);
        if (argv[1] == endptr)
        {
            fprintf(stderr, "Error: %s is not a number\n", argv[1]);
            task.status = -3;
            return -1;
        }
        if (*endptr == '!')
        {
            task.operation = '!';
            return 0;
        }
    }

    task.status = -3;
    return -1;
}

Task* make_task(const char* expression, int& errcode)
{
    if (!expression)
    {
        errcode = 1;
        return nullptr;
    }

    int argc;
    char** argv = parse_arguments_c(expression, &argc);

    if (!argv)
    {
        errcode = 2;
        return nullptr;
    }
    Task* task = static_cast<Task*>(malloc(sizeof(Task)));
    int parse_result = parse(argc, argv, *task);

    // Free argument memory
    free_arguments_c(argv, argc);

    if (parse_result == 0)
    {
        return task;
    }
    else
    {
        errcode = 3;
        free(task);
        return nullptr;
    }
}

const char* error_to_string(int errcode)
{
    static char* NULL_EXPRESSION = "Error! NULL expression!";
    static char* MEM_FAILED = "Error! Memory allocation failed!";
    static char* INVALID_INPUT = "Error! Invalid input!";
    switch (errcode)
    {
        case 1:
            return NULL_EXPRESSION;
            break;
        case 2:
            return MEM_FAILED;
            break;
        case 3:
            return INVALID_INPUT;
            break;
    }
}

} // namespace calculator
