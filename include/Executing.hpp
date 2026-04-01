#pragma once
#include "Command.hpp"
#include "JobControl.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>

using BuiltinFn = ExecResult(*)(const std::vector<std::string>& tokens);
using RedirectionFn = std::pair<int, int>(*)(const std::string&);

namespace CommandExecuting
{
    ExecResult execute_builtin(const Command & cmd);
    bool is_builtin(const std::vector<std::string>& tokens);
    ExecResult handle_execution(const Command& cmd);
    ExecResult execute_foreground(const Command& cmd);
    ExecResult execute_background(const Command& cmd);
    void Restore_file_descriptors();
}
