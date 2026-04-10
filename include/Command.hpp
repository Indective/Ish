#pragma once
#include <iostream>
#include <vector>

enum class ExecResult 
{
    OK,
    EXIT,
    ERROR
};

struct Command
{
    std::vector<std::string> tokens;
<<<<<<< HEAD
    std::vector<std::pair<std::string, std::string>> redirect;
};

struct PipeLine
{
    std::vector<Command> commands;
    bool is_background = false;
};

=======
    bool is_pipe = false;
    std::vector<std::pair<std::string, std::string>> redirect;
};

>>>>>>> db721ab222094a1b1b7d704edd03c43eb265182f
