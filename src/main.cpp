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
    Lexer lex;
    Parser parse;
    ExecResult result = ExecResult::Continue;
    Executor exec;

    shell.init_shell();

    while(result != ExecResult::Exit)
    {   

        const char * input = shell.get_input(shell.build_prompt(shell.get_full_path()).c_str());

        if(JobControl::child_changed)
        {
            JobControl::child_changed = 0;
            JobControl::reap_finished_jobs();
        }

        if(!input) // avoid dangling pointers that the program tries to parse later on (causing core dumps)
        {
            break;
        }
        
        std::optional<std::vector<Token>> tokens = lex.tokenize(input);

        if(!tokens)
        {
            continue;
        }
        
        std::optional<Job> job = parse.Parse_Job(*tokens);
        if(job)
        {
            auto &pipelines = job->chain.pipelines;
            for(auto &pipeline : pipelines)
            {
                for(auto &command : pipeline.commands)
                {
                    shell.replace_alias(command);
                }
            }
            
            result = exec.execute_job(*job);   
        }

    }

    return 0;
}

