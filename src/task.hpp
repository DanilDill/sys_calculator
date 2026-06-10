#pragma once
#include <nlohmann/json.hpp>
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
void from_json(const nlohmann::json& j, Task& t);
void to_json(nlohmann::json& j, const Task& t);
int parse(int argc, char** argv, Task& task);
Task* make_task(const char* expression, int& errcode);
const char* error_to_string(int errcode);
} // namespace calculator
