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
    char* full_path;
    char* input;

    environ.load_aliases(); // pretty self explanitory, load aliases into memory (using an unordered_map)
    signal(SIGCHLD, JobControl::sigchldHandler);

    while(result != ExecResult::EXIT)
    {
        full_path = getcwd(buffer,MAX_BUFFER_LENGTH); 
        std::string shortend_path = environ.shorten_path(full_path); // an obv shit line of code because i cannot find a proper way of converting a std::string to a char * which forces me to use another variable

        input = environ.get_input(shortend_path.c_str());
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

