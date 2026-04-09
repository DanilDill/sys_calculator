#include "app.h"
#include "libmath.h"
#include <execinfo.h>
#include <iostream>

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
			task.result = math::add(task.value1, task.value2);
			break;
		case '-':
			task.result = math::sub(task.value1, task.value2);
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
        case '%':
            task.result = math::mod(task.value1, task.value2, task.status);
            break;
		default:
			task.status = 1;
	}
}

void output(Task task)
{
	if (task.status == 0)
	{
		if (task.operation != '!')
        {
            std::cout << task.value1 << ' ' << task.operation << ' ' << task.value2 << " = " << task.result << '\n';
        }
        else
        {
            std::cout << task.value1 << ' ' << task.operation << ' ' << " = " << task.result << '\n';
        }
        
	}
	else if (task.status == -1)
	{
        fprintf(stderr, "Error! Division by zero!\n");
	}
	else if (task.status == 1)
	{
        fprintf(stderr, "Error! Unknown operation!\n");
	}
    else if (task.status == -2)
	{
        fprintf(stderr, "Error! Overflow!\n");
	}
	else
	{
		std::cout << "Unknown error\n";
	}
}

}

namespace app
{

void run(int argc, char** argv)
{
	Task task;
	if( parse(argc, argv, task) == 0)
    {
        calculate(task);
    }
    else
    {
        fprintf(stderr, "Error! Incorrect arguments!\n");
    }
    output(task);
    
	
}

}