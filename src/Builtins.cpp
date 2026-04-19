#include "Builtins.hpp"
#include <vector>

ExecResult builtin_cd(const std::vector<std::string>& tokens)
{
    if(tokens.size() != 2)
    {
        return ExecResult::Continue;
    }

    const char* dir_name = tokens[1].c_str();

    if(chdir(dir_name) == 0)
    {
        return ExecResult::Continue;
    }

    std::cerr << "cd : Directory not found : " << dir_name << std::endl;

    return ExecResult::Continue;
}

ExecResult builtin_exit(const std::vector<std::string>& tokens)
{
    for(auto& it : JobControl::background_jobs)
    {
        if(it.status == JobStatus::RUNNING)
        {
            std::cout << "Cannot exit, Background processes running" << std::endl;
            return ExecResult::Continue;
        }
    }

    return ExecResult::Exit;
}

ExecResult builtin_jobs(const std::vector<std::string>& tokens)
{
    for(auto& job : JobControl::background_jobs)
    {
        if(job.status == JobStatus::RUNNING)
        {
            rl_on_new_line();

            std::cout << "[" << job.id << "]+\trunning\t";

            for(auto& command : job.commands)
            {
                for(auto& arg : command.argv)
                {
                    std::cout << arg << " ";
                }
            }

            std::cout << std::endl;
        }
    }

    return ExecResult::Continue;
}