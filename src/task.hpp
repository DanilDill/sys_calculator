#pragma once
namespace calculator
{
struct Task
{
    int value1;
    char operation;
    int value2;
    int status;
    int result;
};

int parse(int argc, char** argv, Task& task);
Task* make_task(const char* expression, int& errcode);
const char* error_to_string(int errcode);
} // namespace calculator
