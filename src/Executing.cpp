#include "Executing.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>


namespace CommandExecuting
{
    ExecResult builtin_cd(const Command& cmd);
    ExecResult builtin_exit(const Command& cmd);
    ExecResult builtin_jobs(const Command& cmd);

    static std::unordered_map<std::string, BuiltinFn> builtins =
    {
        {"cd", builtin_cd},
        {"exit", builtin_exit},
        {"jobs",builtin_jobs}
    };

    ExecResult execute_foreground(const Command &cmd)
    {
        std::vector<char*> argv;
        pid_t pid = fork();
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !\n");
            return ExecResult::ERROR;
        }
        else if(pid == 0) // code accessible only by the child process
        {
            for(auto &arg : cmd.args)
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            execvp(argv[0],argv.data());
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
        pid_t pid;
        pid = fork();
        std::vector<char*> argv;
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !\n");
            return ExecResult::ERROR;
        }
        else if(pid == 0) // code accessible only by the child process
        {
            for(auto &arg : cmd.args)
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            execvp(argv[0],argv.data());
            perror("ish");
            _exit(EXIT_FAILURE);
            
        }
        else
        {
            JobControl::job_counter++;
            JobControl::jobs.push_back({JobControl::job_counter,pid,cmd.args, JobStatus::RUNNING});
            std::cout << "[" << JobControl::job_counter << "] " << pid << std::endl; 
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
        if (cmd.args.empty())
        {
            return ExecResult::ERROR;
        }
        auto it = builtins.find(cmd.args[0]);
        return it->second(cmd);
    }

    bool is_builtin(const Command& cmd)
    {
        if (cmd.args.empty())
            return false;

        return builtins.find(cmd.args[0]) != builtins.end();
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
        if (cmd.args.size() != 2)
        {
            return ExecResult::ERROR;
        }
        dir_name = cmd.args[1].c_str();

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
    ExecResult builtin_jobs(const Command &)
    {
        for(auto &job : JobControl::jobs)
        {
            if(job.status == JobStatus::RUNNING)
            {
                rl_on_new_line();
                std::cout << "[" << job.id << "]+" << "\trunning\t";
                for(auto &it : job.command)
                {
                    std::cout << it << " ";
                }
                std::cout << std::endl;
            }
        }
        return ExecResult::OK;
    }
}