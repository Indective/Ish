#pragma once
#include "CommandModel.hpp"
#include "JobControl.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <array>
#include <optional>

enum class ExecResult
{
    Continue,
    Failed,
    Exit
};

class Executor
{
private : 
    ExecResult execute_builtin(const Command & cmd);
    bool is_builtin(const std::vector<std::string>& tokens);
    ExecResult execute_pipe(const Pipeline& p, const bool &is_background);
    ExecResult execute_external_command(const Command& command, const bool &is_background);
    ExecResult execute_chain(const AndChain &chain, const bool &is_background);
    void close_pipes(std::vector<std::array<int, 2>> &pipes);
public : 
    ExecResult execute_job(const Job &job);
};
