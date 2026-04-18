// Parsed command model
//
// Defines the data structures produced by the parser to represent
// shell commands, pipelines, conditional chains, redirections,
// and background execution.

#pragma once
#include "token.hpp"
#include <vector>
#include <string>
#include <optional>

struct Redirection
{
    TokenType op;
    std::string target;

    Redirection(TokenType op, std::string target)
    {
        this->op = op;
        this->target = target;
    }
};

struct Command
{
    std::vector<std::string> argv;
    std::vector<Redirection> redirections;

    Command(std::vector<std::string> argv, std::vector<Redirection> redirections)
    {
        this->argv = argv;
        this->redirections = redirections;
    }
};

struct Pipeline
{
    std::vector<Command> commands;
};

struct AndChain
{
    std::vector<Pipeline> pipelines;
};

struct Job
{
    AndChain chain;
    bool background = false;
};
