#include "Parser.hpp"
#include "Lexer.hpp"
#include "CommandModel.hpp"
#include "Executor.hpp"
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
    Executor exec;

    shell.load_aliases();
    signal(SIGCHLD, JobControl::sigchldHandler);

    while(result != ExecResult::Exit)
    {   
        Lexer lex;
        Parser parse;
        const size_t MAX_BUFFER_LENGTH = 1024;
        char buffer[MAX_BUFFER_LENGTH]; // save the path
        const char* full_path = getcwd(buffer,MAX_BUFFER_LENGTH); 

        const char * input = shell.get_input(shell.build_prompt(full_path).c_str());

        if(!input) // avoid dangling pointers that the program tries to parse later on (causing core dumps)
        {
            break;
        }
        
        std::optional<std::vector<Token>> tokens = lex.tokenize(input);
        std::optional<Job> job = parse.Parse_Job(*tokens);
        auto &pipelines = job->chain.pipelines;
        for(auto &pipeline : pipelines)
        {
            for(auto &command : pipeline.commands)
            {
                shell.replace_alias(command);
            }
        }
        
        result = exec.execute_job(*job);

        JobControl::reap_finished_jobs();
        
    }

    return 0;
}

