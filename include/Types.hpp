#pragma once

#include <vector>
#include <string>
#include <optional>

// execution types
enum class ExecResult
{
    Continue,
    Failed,
    Stopped,
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
enum class ProcessState
{
    RUNNING,
    DONE,
    STOPPED,
    FAILED
};

enum class JobMode
{
    FOREGROUND,
    BACKGRROUND
};

enum class JobState
{
    RUNNING,
    STOPPED,
    DONE
};

struct Process
{
    pid_t pid;
    pid_t pgid;
    ProcessState state;
};

struct JobData
{
    int id;
    std::vector<Process> processes;
    std::vector<Command> commands;
    JobState state;
    JobMode mode;
    int exit_code = 0;

    bool operator==(const JobData& other) const 
    {
        return id == other.id;
    }
};


// alias types 

enum class AliasTokenType
{
    Word,
    EqualSign,
    Value
};

struct AliasToken
{
    AliasTokenType type;
    std::string value;
};

struct Alias
{
    std::string name;
    std::string value;
};