#pragma once
#include <iostream>
#include <vector>
#include <array>

enum class ExecResult 
{
    Exit,
    Continue
};

struct Command
{
    std::vector<std::string> tokens;
    std::vector<std::pair<std::string, std::string>> redirect;

    Command(std::vector<std::string> tokens)
    {
        this->tokens = tokens;
        this->redirect = {};
    }
    Command()
    {
        tokens = {};
        redirect = {};
    }
};

struct PipeLine
{
    std::vector<std::array<int, 2>> pipes;
    std::vector<Command> commands;
    bool is_background = false , is_pipe = false;
};

