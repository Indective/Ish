#pragma once
#include "Types.hpp"
#include "Executor.hpp"

#include <iostream>

using BuiltinFn_Exec = ExecResult(*)(const std::vector<std::string>& argv);
using BuiltinFn_JobControl = ExecResult(*)(std::list<JobData>::iterator job);

ExecResult builtin_cd(const std::vector<std::string>& argv);
ExecResult builtin_exit(const std::vector<std::string>& argv);
ExecResult builtin_jobs(const std::vector<std::string>& argv);
ExecResult builtin_source(const std::vector<std::string>& argv);


ExecResult builtin_bg(std::list<JobData>::iterator job);
ExecResult builtin_fg(std::list<JobData>::iterator job);

static std::unordered_map<std::string, BuiltinFn_Exec> builtins_exec =
{
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"jobs", builtin_jobs},
    {"source", builtin_source}
};

static std::unordered_map<std::string, BuiltinFn_JobControl> builtins_jobcontrol =
{
    {"fg", builtin_fg},
    {"bg", builtin_bg}
};