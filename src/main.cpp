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

    environ.load_aliases(); // pretty self explanitory, load aliases into memory (using an unordered_map)
    signal(SIGCHLD, JobControl::sigchldHandler);

    while(result != ExecResult::EXIT)
    {
        temp = getcwd(buffer,MAX_BUFFER_LENGTH); 
        std::string path(temp);
        path = environ.shorten_path(path);

        input = environ.get_input(path.c_str());
        if(!input)
        {
            break;
        }

        command.args = CommandParsing::parse_command(input);
        command.is_background = JobControl::handle_background(command.args);

        environ.replace_alias(command.args); // look for aliases, if found, replace them in command.arg
        result = CommandExecuting::handle_execution(command, command.is_background);

        JobControl::reap_finished_jobs();
        free(input); // (hopefully) avoid segfaults
    }

    return 0;
}

