#include "Parsing.hpp"
#include "Executing.hpp"
#include "ShellContext.hpp"
#include "JobControl.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <optional>

int main()
{
    ShellContext shell;
    ExecResult result = ExecResult::Continue;

    shell.load_aliases();
    signal(SIGCHLD, JobControl::sigchldHandler);

    while(result != ExecResult::Exit)
    {   
        const size_t MAX_BUFFER_LENGTH = 1024;
        char buffer[MAX_BUFFER_LENGTH]; // save the path
        const char* full_path = getcwd(buffer,MAX_BUFFER_LENGTH); 

        const char * input = shell.get_input(shell.build_prompt(full_path).c_str());

        if(!input) // avoid dangling pointers that the program tries to parse later on (causing core dumps)
        {
            break;
        }
        
        std::optional<PipeLine> p = CommandParsing::parse_command(input);  
        
        if(p)
        {
            result = CommandExecuting::handle_execution(p);
        }

        JobControl::reap_finished_jobs();
        
    }

    return 0;
}

