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
    Exit
};

using BuiltinFn = ExecResult(*)(const std::vector<std::string>& tokens);
using RedirectionFn = void(*)(const std::string&);

namespace CommandExecuting
{
    ExecResult execute_builtin(const Command & cmd);
    bool is_builtin(const std::vector<std::string>& tokens);
    ExecResult execute_pipe(std::optional<Pipeline>& p);
    ExecResult execute_external(const Command& command, const bool &is_background);
    ExecResult handle_execution(std::optional<Pipeline>& p);
    void close_pipes(std::optional<Pipeline>& p);
}
