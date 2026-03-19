#include "Parsing.hpp"
#include "Executing.hpp"
#include "Environment.hpp"
#include "JobControl.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <string>
#include <unistd.h>

int main()
{
    Environment environ;
    Command command;
    ExecResult result = ExecResult::OK;

    const size_t MAX_BUFFER_LENGTH = 1024;
    char buffer[MAX_BUFFER_LENGTH]; // save the path
    char* temp;
    char* input;
    bool is_background = false;

    environ.load_aliases(); // pretty self explanitory, load aliases into memory (using an unordered_map)

    while(result != ExecResult::EXIT)
    {
        temp = getcwd(buffer,MAX_BUFFER_LENGTH); 

        std::string path(temp);
        path = environ.shorten_path(path);
        input = readline(path.c_str());

        if(!input) // input hit EoF (exit cleanly when hitting Ctrl + D)
        {
            std::cout << "exiting from ctrl d " << std::endl;
            result  = ExecResult::EXIT;
        }

        if(*input) // input is not a nullptr, IOW input is pointing to a character
        {
            add_history(input); // add input to the scrollable history (using Arrow keys)
        }

        command.arg = CommandParsing::parse_command(input); // parse command line 
        if(JobControl::is_background(command.arg))
        {
            command.arg = CommandParsing::remove_background_symbol(command.arg);
            is_background = true;
        }
        else
        {
            is_background = false;
        }

        environ.replace_alias(command.arg); // look for aliases, if found, replace them in command.arg
        result = CommandExecuting::handle_execution(command, is_background);

        signal(SIGCHLD, JobControl::sigchldHandler);
        if(JobControl::childchanged)
        {
            JobControl::childchanged = 0;
            JobControl::print_done_message_and_reap();
        }
        free(input); // (hopefully) avoid segfaults
    }

    return 0;
}

