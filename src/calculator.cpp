// #include "app.h"
// #include "args_parser.h"
#include "libmath.h"
#include "task.hpp"

#include <execinfo.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
namespace
{

void calculate(calculator::Task* task)
{
    task->status = 0;
    switch (task->operation)
    {
        case '+':
            task->result = math::add(task->value1, task->value2, task->status);
            break;
        case '-':
            task->result = math::sub(task->value1, task->value2, task->status);
            break;
        case '*':
            task->result = math::mul(task->value1, task->value2, task->status);
            break;
        case '/':
            task->result = math::div(task->value1, task->value2, task->status);
            break;
        case '^':
            task->result = math::pow(task->value1, task->value2, task->status);
            break;
        case '!':
            task->result = math::factorial(task->value1, task->status);
            break;
        default:
            task->status = 1;
    }
}

} // namespace

namespace calculator
{
char* output(calculator::Task* task)
{
    // Allocate buffer with sufficient size
    char* buffer = static_cast<char*>(malloc(256 * sizeof(char)));
    if (!buffer)
    {
        return nullptr;
    }

    if (task->status == 0)
    {
        if (task->operation != '!')
        {
            snprintf(buffer, 256, "%d %c %d = %d", task->value1,
                     task->operation, task->value2, task->result);
        }
        else
        {
            snprintf(buffer, 256, "%d %c = %d", task->value1, task->operation,
                     task->result);
        }
    }
    else if (task->status == -1)
    {
        snprintf(buffer, 256, "Error! Division by zero!");
    }
    else if (task->status == 1)
    {
        snprintf(buffer, 256, "Error! Unknown operation!");
    }
    else if (task->status == -2)
    {
        snprintf(buffer, 256, "Error! Overflow!");
    }
    else if (task->status == -3)
    {
        snprintf(buffer, 256, "Error! Incorrect arguments!");
    }
    else
    {
        snprintf(buffer, 256, "Unknown error");
    }

    return buffer;
}

void run(calculator::Task* task)
{
    calculate(task);
}

} // namespace calculator