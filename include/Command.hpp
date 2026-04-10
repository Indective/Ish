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
    std::vector<Command> commands;
    bool is_background = false;
};

