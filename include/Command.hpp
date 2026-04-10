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
    std::vector<std::pair<std::string, std::string>> redirect;
};

struct PipeLine
{
    std::vector<Command> commands;
    bool is_background = false;
};

