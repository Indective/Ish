#include "Parsing.hpp"
#include "Executing.hpp"
#include "Environment.hpp"
#include "JobControl.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <optional>

int main()
{
    Environment environ;
    ExecResult result = ExecResult::OK;

    const size_t MAX_BUFFER_LENGTH = 1024;
    char buffer[MAX_BUFFER_LENGTH]; // save the path
    char* full_path;
    char* input;

    environ.load_aliases();
    signal(SIGCHLD, JobControl::sigchldHandler);

    while(result != ExecResult::EXIT)
    {
        
        full_path = getcwd(buffer,MAX_BUFFER_LENGTH); 

        input = environ.get_input(environ.build_prompt(full_path).c_str());

        if(!input) // avoid dangling pointers that the program tries to parse later on (causing core dumps)
        {
            break;
        }

        std::optional<PipeLine> p = CommandParsing::parse_command(input);  
        
        if(pipe)
        {
            for(auto &it : p->commands)
            {
                environ.replace_alias(it); // exppand aliases in all commands 
            }
            result = CommandExecuting::handle_execution(p);
        }

        JobControl::reap_finished_jobs();
        
        free(input); // (hopefully) avoid segfaults
    }

    return 0;
}

