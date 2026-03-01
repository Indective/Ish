#include "Parsing.hpp"
#include "Executing.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

int main()
{
    Command command;
    std::string line;
    const size_t MAX_BUFFER_LENGTH = 1024;
    char buffer[MAX_BUFFER_LENGTH];
    while(1)
    {
        std::cout << getcwd(buffer,MAX_BUFFER_LENGTH) << " $ ";
        std::getline(std::cin, line);
        command.arg = CommandParsing::parse_command(line);
        if(CommandExecuting::is_builtin(command))
        {
            if(CommandExecuting::execute_builtin(command) == ExecResult::EXIT)
            {
                break;
            }
        }
        else
        {
            CommandExecuting::execute_external(command);
        }
    }

    return 0;
}

