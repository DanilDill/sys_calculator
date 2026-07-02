#pragma once
#include <nlohmann/json.hpp>



namespace calculator
{

struct Task
{
/// Status of Task
enum class Status : int
{
    INCORRECT_ARGUMENTS = -3,
    OVERFLOW = -2,
    DIV_BY_ZERO = -1,
    OK = 0,
    UNKNOWN_OPERATION = 1,
    UNKNOWN = 2
};
/// @brief Convert status to string
/// @param status 
/// @return string of status
static std::string to_string(const Task::Status status);

/// @brief operation may be + - * / !
char operation;

/// @brief status variable of operation
Task::Status status;
/// @brief first value
int value1;

/// @brief second value
int value2;   

/// @brief result of operation
int result;

};
/// @brief  Func for converting json to Task with nlohmann
/// @param j json string 
/// @param t Task reference
void from_json(const nlohmann::json& j, Task& t);

/// @brief Func for converting Task to json with nlohmann
/// @param j json string 
/// @param t Task reference
void to_json(nlohmann::json& j, const Task& t);
};



