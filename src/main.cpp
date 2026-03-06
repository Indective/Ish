#include "Parsing.hpp"
#include "Executing.hpp"
#include "Environment.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

int main()
{
    Environment environ;
    Command command;
    std::string line;
    const size_t MAX_BUFFER_LENGTH = 1024;
    char buffer[MAX_BUFFER_LENGTH];

    environ.load_aliases(); // pretty self explanitory, load aliases into memory (using an unordered_map)

    while(1)
    {
        std::cout << getcwd(buffer,MAX_BUFFER_LENGTH) << " $ ";
        std::getline(std::cin, line);
        command.arg = CommandParsing::parse_command(line); // parse command line 
        environ.replace_alias(command.arg); // look for aliases, if found, replace them in command.arg

        if(CommandExecuting::is_builtin(command))
        {
            if(CommandExecuting::execute_builtin(command) == ExecResult::EXIT)
            {
                break;
            }
        }
        else
        {
            if(!(CommandExecuting::execute_external(command) == ExecResult::OK))
            {
                std::cout << "Command execution failed !" << std::endl;
            }
        }
    }

    return 0;
}

