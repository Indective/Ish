#pragma once
#include "Types.hpp"
#include "Executor.hpp"

#include <iostream>

using BuiltinFn = ExecResult(*)(const std::vector<std::string>& argv);

ExecResult builtin_cd(const std::vector<std::string>& argv);
ExecResult builtin_exit(const std::vector<std::string>& argv);
ExecResult builtin_jobs(const std::vector<std::string>& argv);
ExecResult builtin_source(const std::vector<std::string>& argv);

static std::unordered_map<std::string, BuiltinFn> builtins =
{
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"jobs", builtin_jobs},
    {"source", builtin_source}
};