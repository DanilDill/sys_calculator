// #include "app.h"
// #include "args_parser.h"
#include "calculator.hpp"
#include "libmath.h"


#include <execinfo.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace calculator
{

void Calculator::execute(Task& task)
{
    int err = 0;
    switch (task.operation)
    {
        case '+':
            task.result = math::add(task.value1, task.value2,err);
            break;
        case '-':
            task.result = math::sub(task.value1, task.value2, err);
            break;
        case '*':
            task.result = math::mul(task.value1, task.value2, err);
            break;
        case '/':
            task.result = math::div(task.value1, task.value2, err);
            break;
        case '^':
            task.result = math::pow(task.value1, task.value2, err);
            break;
        case '!':
            task.result = math::factorial(task.value1, err);
            break;
        default:
            err = -3;
    }
    task.status = static_cast<calculator::Task::Status>(err);
}

} // namespace calculator