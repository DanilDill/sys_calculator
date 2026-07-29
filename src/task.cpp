
#include "task.hpp"

#include <cstdio>
#include <cstdlib>

namespace calculator
{

void from_json(const nlohmann::json& j, Task& t)
{
    j.at("firstValue").get_to(t.value1);
    std::string op = j.at("operation").get<std::string>();
    if (op.size() != 1)
    {
        throw std::invalid_argument("operation must be a single character");
    }
    t.operation = op[0];
    t.value2 = 0;
    if (t.operation != '!')
    {
        j.at("secondValue").get_to(t.value2);
    }
    t.result = 0;
    t.status = Task::Status::OK;
}
void to_json(nlohmann::json& j, const Task& t)
{
    j = nlohmann::json{{"firstValue", t.value1},
                       {"operation", std::string(1, t.operation)},
                       {"secondValue", t.value2},
                       {"status", Task::to_string(t.status)}};
    if (t.status == Task::Status::OK)
    {
        j["result"] = t.result;
    }
}

std::string Task::to_string(Task::Status status)
{
    using Status = Task::Status;
    switch (status)
    {
        case Status::OK:
            return "success";
        case Status::DIV_BY_ZERO:
            return "Error! Division by zero!";
        case Status::OVERFLOW:
            return "Error! Overflow!";
        case Status::UNKNOWN:
            return "UNKNOWN STATUS";
        case Status::INCORRECT_ARGUMENTS:
            return "Error! Incorrect arguments!";
        default:
            return "Error! Unknown error!";
    }
}

} // namespace calculator
