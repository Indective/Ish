#include "Parsing.hpp"
#include <iostream>
#include <string>

int main()
{
    Command command;
    std::string line = "this is a line command";
    command.arg = parsing::parse_command(line);

    for(auto& i : command.arg)
    {
        std::cout << i << std::endl;
    }
    return 0;
}

