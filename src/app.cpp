#include "app.h"
#include "args_parser.h"
#include "libmath.h"
#include <execinfo.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>

namespace
{
struct Task
{
	int value1;
	char operation;
	int value2;
	int status;
	int result;
};

int parse(int argc, char** argv, Task& task)
{
    if(argc ==4)
    {
        task.value1 = atoi(argv[1]);
	    task.operation = *(argv[2]);
	    task.value2 = atoi(argv[3]);
        return 0;
    }
    else if (argc == 2)
    {
        char* endptr;
        task.value1 = strtol(argv[1],&endptr,10);
        if (argv[1] == endptr)
        {
            fprintf(stderr, "Error: %s is not a number\n", argv[1]);
            task.status = -3;
            return -1;
        }
        if (*endptr =='!')
        {
            task.operation = '!';
            return 0;
        }
        
    }
    
    task.status = -3;
    return -1;
	
}

void calculate(Task& task)
{
	task.status = 0;
	switch(task.operation)
	{
		case '+':
			task.result = math::add(task.value1, task.value2, task.status);
			break;
		case '-':
			task.result = math::sub(task.value1, task.value2,task.status);
			break;
		case '*':
			task.result = math::mul(task.value1, task.value2,task.status);
			break;
		case '/':
			task.result = math::div(task.value1, task.value2, task.status);
			break;
		case '^':
			task.result = math::pow(task.value1, task.value2, task.status);
			break;
        case '!':
            task.result = math::factorial(task.value1, task.status); 
            break;
		default:
			task.status = 1;
	}
}

char* output(Task task)
{
    // Allocate buffer with sufficient size
    char* buffer = static_cast<char*>(malloc(256 * sizeof(char)));
    if (!buffer) {
        return nullptr;
    }
    
	if (task.status == 0)
	{
		if (task.operation != '!')
        {
            snprintf(buffer, 256, "%d %c %d = %d", task.value1, task.operation, task.value2, task.result);
        }
        else
        {
            snprintf(buffer, 256, "%d %c = %d", task.value1, task.operation, task.result);
        }
	}
	else if (task.status == -1)
	{
        snprintf(buffer, 256, "Error! Division by zero!");
	}
	else if (task.status == 1)
	{
        snprintf(buffer, 256, "Error! Unknown operation!");
	}
    else if (task.status == -2)
	{
        snprintf(buffer, 256, "Error! Overflow!");
	}
    else if (task.status == -3)
    {
        snprintf(buffer, 256, "Error! Incorrect arguments!");
    }
	else
	{
		snprintf(buffer, 256, "Unknown error");
	}
    
    return buffer;
}

}

namespace app
{

char* run(const char* expression)
{
    if (!expression) {
        char* error = static_cast<char*>(malloc(256 * sizeof(char)));
        if (error) {
            snprintf(error, 256, "Error! NULL expression!");
        }
        return error;
    }
    
	Task task;
    
    // Parse expression into command-line arguments
    int argc;
    char** argv = parse_arguments_c(expression, &argc);
    
    if (!argv) {
        char* error = static_cast<char*>(malloc(256 * sizeof(char)));
        if (error) {
            snprintf(error, 256, "Error! Memory allocation failed!");
        }
        return error;
    }
    
    int parse_result = parse(argc, argv, task);
    
    // Free argument memory
    free_arguments_c(argv, argc);
    
    if (parse_result == 0) {
        calculate(task);
    } else {
        // parse already set task.status for incorrect arguments
    }
    
    return output(task);
}

}