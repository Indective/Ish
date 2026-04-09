#include "Parsing.hpp"
#include "Executing.hpp"
#include "Environment.hpp"
#include "JobControl.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <string>
#include <unistd.h>

// TO DO : (timeline)
// Feat : Implement piping
// Refactor : merge the 2 Execute functions into one (exec fg and exec bg)
// Refactor : ExecResult is becoming redundant
// Refactor : Treat a foreground proccess and a background proccess as one job in JobControl
// Refactor : refactor and improve job control

int main()
{
    Environment environ;
    ExecResult result = ExecResult::OK;

    const size_t MAX_BUFFER_LENGTH = 1024;
    char buffer[MAX_BUFFER_LENGTH]; // save the path
    char* full_path;
    char* input;

    environ.load_aliases(); // pretty self explanitory, load aliases into memory (using an unordered_map)
    signal(SIGCHLD, JobControl::sigchldHandler);

    while(result != ExecResult::EXIT)
    {
        Command command;
        full_path = getcwd(buffer,MAX_BUFFER_LENGTH); 

        input = environ.get_input(environ.build_prompt(full_path).c_str());

        if(!input) // avoid dangling pointers that the program tries to parse later on (causing core dumps)
        {
            break;
        }

        command.tokens = CommandParsing::parse_command(input, command);        
        if(!command.tokens.empty())
        {
            environ.replace_alias(command); // look for aliases, if found, replace them in command.arg
            result = CommandExecuting::handle_execution(command);
        }

        JobControl::reap_finished_jobs();
        free(input); // (hopefully) avoid segfaults
    }

    return 0;
}

