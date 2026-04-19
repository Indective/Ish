#pragma once
#include "CommandModel.hpp"
#include "Executor.hpp"

#include <iostream>

using BuiltinFn = ExecResult(*)(const std::vector<std::string>& tokens);

ExecResult builtin_cd(const std::vector<std::string>& tokens);
ExecResult builtin_exit(const std::vector<std::string>& tokens);
ExecResult builtin_jobs(const std::vector<std::string>& tokens);

static std::unordered_map<std::string, BuiltinFn> builtins =
{
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"jobs", builtin_jobs}
};