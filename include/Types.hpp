#pragma once

#include <vector>
#include <string>
#include <optional>

// execution types
enum class ExecResult
{
    Continue,
    Failed,
    Exit
};

// command ast types
enum class TokenType 
{
    Word,
    Pipe,
    AndAnd,
    Ampersand,
    STDOUT_APPEND, // >>
    STDOUT_OVERWRITE, // >
    STDIN_HEREDOC, // << 
    STDIN_HERESTRING,// <<<
    STDERR, // 2>
    STDIN // < 
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType type, std::string value)
    {
        this->type = type;
        this->value = value;
    }
    Token(TokenType type)
    {
        this->type = type;
        this->value = "";
    }
    Token()
    {
        this->value = "";
    }
};

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


// job control types
enum class State
{
    RUNNING,
    DONE,
    STOPPED,
    FAILED
};

struct Process
{
    pid_t pid;
    State state;
};

struct JobData
{
    int id;
    std::vector<Process> processes;
    std::vector<Command> commands;
    bool is_done = false;
    bool is_stopped = false;
};