#include "Executing.hpp"
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

    ExecResult execute_foreground(const Command &cmd)
    {
        std::vector<char*> arg_c;
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
            waitpid(pid,nullptr,0);
            return ExecResult::OK;
        }
        return ExecResult::ERROR;

    }

    ExecResult execute_background(const Command &cmd)
    {
        std::cout << "running background" << std::endl;
        pid_t pid;
        pid = fork();
        std::vector<char*> arg_c;
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
            JobControl::jobs.push_back({JobControl::job_counter++,pid,cmd.arg, JobStatus::RUNNING});
            return ExecResult::OK;
        }
    }

    ExecResult handle_external(const Command &cmd, const bool &is_background)
    {
        if(is_background)
        {
            return execute_background(cmd);
        }
        else
        {
            return execute_foreground(cmd);
        }
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

    ExecResult handle_execution(const Command &cmd,const bool &is_background)
    {
        if(CommandExecuting::is_builtin(cmd))
        {
            return CommandExecuting::execute_builtin(cmd);
        }

        else
        {
            return CommandExecuting::handle_external(cmd, is_background);
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
        for(auto &it : JobControl::jobs)
        {
            if(it.status == JobStatus::RUNNING)
            {
                std::cout << "Cannot exit, Background proccesses running" << std::endl;
                return ExecResult::OK;
            }
        }
        return ExecResult::EXIT;
    }
}