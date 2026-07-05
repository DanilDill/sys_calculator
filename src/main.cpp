
#include "app.hpp"
#include <logger.hpp>
#include <sw/redis++/redis++.h>
#include <libpq-fe.h>
#include <iostream>
int main(int argc, char** argv)
{
    try
    {
        calculator::app(argc,argv).run();
        return 0;
    }
    catch(const std::exception& e)
    {
        Logger::instance().error(e.what());
    }
    // try {
    //     sw::redis::Redis redis("tcp://127.0.0.1:6379");

    //     redis.set("greeting", "hello");
    //     auto val = redis.get("greeting");   // std::optional<std::string>

    //     if (val)
    //         std::cout << "GET greeting -> " << *val << '\n';
    //     else
    //         std::cout << "key not found\n";

    // } catch (const sw::redis::Error &e) {
    //     std::cerr << "redis error: " << e.what() << '\n';
    //     return 1;
    // }
    return 0;
}
