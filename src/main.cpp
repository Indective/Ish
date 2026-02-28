#include "Parsing.hpp"
#include "Executing.hpp"
#include <iostream>
#include <string>

int main()
{
    Command command;
    std::string line;

    while(1)
    {
        std::cout << "$ ";
        std::getline(std::cin, line);
        command.arg = CommandParsing::parse_command(line);
        if(CommandExecuting::is_builtin(command))
        {
            if(CommandExecuting::execute_builtin(command) == ExecResult::EXIT)
            {
                break;
            }
        }
    }

    return 0;
}

