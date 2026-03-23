#pragma once
#include "Command.hpp"
#include "JobControl.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>

using BuiltinFn = ExecResult(*)(const Command&);

namespace CommandExecuting
{
    ExecResult handle_external(const Command& cmd, const bool &is_background);
    ExecResult execute_builtin(const Command& cmd);
    bool is_builtin(const Command& cmd);
    ExecResult handle_execution(const Command& cmd, const bool &is_background);
    ExecResult execute_foreground(const Command& cmd);
    ExecResult execute_background(const Command& cmd);
}
