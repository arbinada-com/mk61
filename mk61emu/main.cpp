#include <iostream>
#include "mk61commander.h"

int main()
{
    try
    {
        mk61_commander cmd;
        cmd.run();
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
