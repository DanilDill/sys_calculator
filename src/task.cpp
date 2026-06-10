
#include "task.hpp"

#include "args_parser.hpp"

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
    if(t.operation != '!')
    {
        j.at("secondValue").get_to(t.value2);
    }
    t.result = 0;
    t.status = 0;
    if(t.operation =='/' && t.value2 == 0)
    {
        throw std::invalid_argument("Divizion by zero");
    }
}
void to_json(nlohmann::json& j, const Task& t)
{
    j = nlohmann::json{
        {"firstValue",  t.value1},
        {"operation",   std::string(1, t.operation)}, // char -> string
        {"secondValue", t.value2},
        {"result",      t.result},
        {"status",      (t.status == 0) ? "success" : "error"}
    };
}

} // namespace calculator
