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
    std::vector<std::string> args;
    bool is_background = false;
};

