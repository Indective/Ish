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
    bool is_pipe = false;
    std::vector<std::pair<std::string, std::string>> redirect;
};

