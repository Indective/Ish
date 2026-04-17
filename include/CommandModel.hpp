// Parsed command model
//
// Defines the data structures produced by the parser to represent
// shell commands, pipelines, conditional chains, redirections,
// and background execution.

#pragma once
#include "token.hpp"
#include <vector>
#include <string>

struct Redirection
{
    TokenType op;
    std::string target;
};

struct Command
{
    std::vector<std::string> argv;
    std::vector<Redirection> redirections;
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