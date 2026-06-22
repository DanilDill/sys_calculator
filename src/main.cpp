
#include "app.hpp"
#include <logger.hpp>
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
    
}
