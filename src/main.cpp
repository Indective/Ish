#include "Parsing.hpp"
#include "Executing.hpp"
#include "Environment.hpp"
#include <readline/readline.h>
#include <readline/history.h>
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
    char* temp;
    char* input;

    environ.load_aliases(); // pretty self explanitory, load aliases into memory (using an unordered_map)

    while(1)
    {
        temp = getcwd(buffer,MAX_BUFFER_LENGTH);

        std::string path(temp);
        path = environ.shorten_path(path);
        input = readline(path.c_str());

        if(!input)
        {
            break;
        }
        if(*input)
        {
            add_history(input);
        }

        command.arg = CommandParsing::parse_command(input); // parse command line 
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

        free(input);
    }

    return 0;
}

