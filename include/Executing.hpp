#pragma once
#include "Parsing.hpp"
#include <iostream>
#include <unordered_map>

using BuiltinFn = ExecResult(*)(const Command&);

namespace CommandExecuting
{
    ExecResult execute_internal(const Command& cmd);
    ExecResult execute_builtin(const Command& cmd);
    bool is_builtin(const Command& cmd);
}
