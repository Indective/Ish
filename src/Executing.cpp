#include "Executing.hpp"
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

namespace CommandExecuting
{
    static ExecResult builtin_cd(const Command& cmd);
    static ExecResult builtin_exit(const Command& cmd);

    static std::unordered_map<std::string, BuiltinFn> builtins =
    {
        {"cd", builtin_cd},
        {"exit", builtin_exit}
    };

    ExecResult execute_external(const Command &cmd)
    {
        std::vector<char*> arg_c;
        pid_t pid = fork();
        if(pid == -1)
        {
            perror("Failed to fork process !\n");
            return ExecResult::ERROR;
        }
        else if(pid == 0)
        {
            for(auto &it : cmd.arg)
            {
                arg_c.push_back(const_cast<char*>(it.c_str()));
            }
            arg_c.push_back(nullptr);
            execvp(arg_c[0],arg_c.data());
            return ExecResult::ERROR;
        }
        else
        {
            waitpid(pid,nullptr,0);
            return ExecResult::OK;
        }
        return ExecResult::OK;

    }

    ExecResult execute_builtin(const Command &cmd)
    {
        if (cmd.arg.empty())
        {
            return ExecResult::ERROR;
        }
        auto it = builtins.find(cmd.arg[0]);
        return it->second(cmd);
    }

    bool is_builtin(const Command& cmd)
    {
        if (cmd.arg.empty())
            return false;

        return builtins.find(cmd.arg[0]) != builtins.end();
    }

    //Builtin implementations

    static ExecResult builtin_cd(const Command& cmd)
    {
        const char* dir_name;
        if (cmd.arg.size() != 2)
        {
            return ExecResult::ERROR;
        }
        dir_name = cmd.arg[1].c_str();

        if (chdir(dir_name) == 0)
        {
            return ExecResult::OK;
        }
        else 
        {
           std::cerr << "cd : Directory not found : " << dir_name << std::endl; 
        }
        
        return ExecResult::ERROR;
    }

    static ExecResult builtin_exit(const Command&)
    {
        return ExecResult::EXIT;
    }
}