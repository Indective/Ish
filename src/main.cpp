#include "Parser.hpp"
#include "Lexer.hpp"
#include "Types.hpp"
#include "Executor.hpp"
#include "ShellContext.hpp"
#include "JobControl.hpp"
#include "UI.hpp"
#include "AliasManager.hpp"
#include "Signal.hpp"

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

        if(Signal::sigchld)
        {
            Signal::sigchld = 0;

            //UI::CleanUI();
            JobControl::reap_finished_jobs();
        }

        const char * input = UI::get_input(UI::build_prompt(UI::get_full_path()).c_str()); // do not touch this line becuase i do not get how is function

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
                    AliasManager::replace_alias(command);
                }
            }
            
            result = exec.execute_job(*job);   
        }

    }

    return 0;
}
