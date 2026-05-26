#include "UI.hpp"
#include "readline/readline.h"
#include "readline/history.h"

#include <iostream>
#include <unistd.h>

namespace UI
{
    static constexpr const char* postfix = " $ "; // ui
    static constexpr const char* blue = "\033[34m"; // ui
    static constexpr const char* reset = "\033[0m"; // ui

    std::string shorten_path(const std::string &path)
    {
        const char* home = getenv("HOME");
        if (home && path.rfind(home, 0) == 0) // check if path starts with $HOME
        {
            return "~" + path.substr(strlen(home));
        }
        return path;
    }

    void CleanUI()
    {
        std::cout << "\n";
        rl_on_new_line();
        rl_redisplay();
    }

    std::string build_prompt(const std::string &path)
    {
        std::string short_path = shorten_path(path);
        std::string prompt = blue + short_path + reset + postfix;
        return prompt; 
    }

    const char *get_input(const char *path)
    {
        char * input;
        while(true)
        {
            input = readline(path);
            if(!input)
            {
                break;
            }
            else if(input[0] != '\0')
            {
                add_history(input);
                return input;
            }
            free(input);
        }
        free(input);
        input = nullptr;
        return input;
    }

    const char *get_full_path()
    {
        const size_t MAX_BUFFER_LENGTH = 1024;
        char buffer[MAX_BUFFER_LENGTH]; // save the path
        return getcwd(buffer,MAX_BUFFER_LENGTH); 
    }
}
