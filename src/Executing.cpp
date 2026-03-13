#include "Executing.hpp"
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

namespace CommandExecuting
{
    ExecResult builtin_cd(const Command& cmd);
    ExecResult builtin_exit(const Command& cmd);

    static std::unordered_map<std::string, BuiltinFn> builtins =
    {
        {"cd", builtin_cd},
        {"exit", builtin_exit}
    };

    ExecResult execute_external(const Command &cmd)
    {
        std::vector<char*> arg_c;
        int status;
        pid_t pid = fork();
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !\n");
            return ExecResult::ERROR;
        }
        else if(pid == 0) // code accessible only by the child process
        {
            for(auto &it : cmd.arg)
            {
                arg_c.push_back(const_cast<char*>(it.c_str()));
            }
            arg_c.push_back(nullptr);
            execvp(arg_c[0],arg_c.data());
            perror("ish");
            _exit(EXIT_FAILURE);
            
        }
        else
        {
            waitpid(pid,&status,0);
            return ExecResult::OK;
        }
        return ExecResult::ERROR;

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

    ExecResult handle_execution(const Command &cmd)
    {
        if(CommandExecuting::is_builtin(cmd))
        {
            return CommandExecuting::execute_builtin(cmd);
        }

        else
        {
            return CommandExecuting::execute_external(cmd);
        }

        std::cout << "handle command error " << std::endl;
    }


    //Builtin implementations

    ExecResult builtin_cd(const Command& cmd)
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

    ExecResult builtin_exit(const Command&)
    {
        return ExecResult::EXIT;
    }
}