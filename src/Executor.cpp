#include "Executor.hpp"
#include "Parser.hpp"
#include "CommandModel.hpp"
#include "Redirection.hpp"
#include "Builtins.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <fcntl.h>
#include <algorithm>
#include <iterator>
#include <optional>


ExecResult Executor::execute_chain(const AndChain& chain, const bool& is_background)
{
    ExecResult result = ExecResult::Continue;

    for(size_t i = 0; i < chain.pipelines.size(); i++)
    {
        result = execute_pipe(chain.pipelines[i], is_background);
        if(result == ExecResult::Failed)
        {
            return result;
        }
    }

    return result;
}

void Executor::close_pipes(std::vector<std::array<int, 2>>& pipes)
{
    for(auto& it : pipes)
    {
        close(it[0]);
        close(it[1]);
    }
}

ExecResult Executor::execute_job(const Job& job)
{
    return execute_chain(job.chain, job.background);
}

ExecResult Executor::execute_builtin(const Command& cmd)
{
    for(auto& [op, fn] : cmd.redirections)
    {
        redircetion_handler[op](fn);
    }

    auto it = builtins.find(cmd.argv[0]);

    if(it == builtins.end())
    {
        return ExecResult::Continue;
    }

    return it->second(cmd.argv);
}

bool Executor::is_builtin(const std::vector<std::string>& tokens)
{
    return builtins.find(tokens[0]) != builtins.end();
}

ExecResult Executor::execute_pipe(const Pipeline& p, const bool& is_background)
{
    if(p.commands.size() == 1)
    {
        if(is_builtin(p.commands[0].argv) && !is_background)
        {
            return execute_builtin(p.commands[0]);
        }
        else
        {
            return execute_external_command(p.commands[0], is_background);
        }
    }
    else
    {
        std::vector<std::array<int, 2>> pipes;
        size_t cmd_count = p.commands.size();
        pid_t pgid = 0;
        std::vector<pid_t> pids;

        pids.reserve(cmd_count);
        pipes.resize(cmd_count - 1);

        for(size_t i = 0; i < pipes.size(); i++)
        {
            if(pipe(pipes[i].data()) == -1)
            {
                perror("pipe");
                return ExecResult::Continue;
            }
        }

        for(size_t i = 0; i < cmd_count; i++)
        {
            pid_t pid = fork();

            if(pid == -1)
            {
                perror("fork");
                return ExecResult::Continue;
            }
            else if(pid == 0)
            {
                if(i == 0)
                {
                    setpgid(0,0);
                }
                else
                {
                    setpgid(0, pgid);
                }

                if(i > 0)
                {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }

                if(i < cmd_count - 1)
                {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                for(auto& [op, fn] : p.commands[i].redirections)
                {
                    redircetion_handler[op](fn);
                }

                close_pipes(pipes);

                std::vector<char*> argv;

                for(auto& arg : p.commands[i].argv)
                {
                    argv.push_back(const_cast<char*>(arg.c_str()));
                }

                argv.push_back(nullptr);

                execvp(argv[0], argv.data());
                perror("execvp");
                _exit(EXIT_FAILURE);
            }
            else
            {
                if(i == 0)
                {
                    pgid = pid;
                }
                setpgid(pid, pgid);

                pids.push_back(pid);

                if(i > 0)
                {
                    close(pipes[i - 1][0]);
                }

                if(i < cmd_count - 1)
                {
                    close(pipes[i][1]);
                }
            }
        }

        if(is_background)
        {
            JobControl::background_jobs.push_back(
            {
                ++JobControl::job_counter,
                pids,
                p.commands,
                JobStatus::RUNNING
            });

            std::cout << "[" << JobControl::job_counter << "]";

            for(auto& pid : pids)
            {
                std::cout << " " << pid;
            }

            std::cout << std::endl;

            return ExecResult::Continue;
        }
        else
        {
            for(pid_t pid : pids)
            {
                waitpid(pid, nullptr, 0);
            }

            return ExecResult::Continue;
        }
    }
}

ExecResult Executor::execute_external_command(const Command& command, const bool& is_background)
{
    std::vector<char*> argv;

    pid_t pid = fork();

    if(pid == -1)
    {
        perror("fork");
        return ExecResult::Continue;
    }
    else if(pid == 0)
    {
        for(auto& arg : command.argv)
        {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }

        argv.push_back(nullptr);

        for(auto& [op, fn] : command.redirections)
        {
            redircetion_handler[op](fn);
        }

        execvp(argv[0], argv.data());
        perror("ish");
        _exit(EXIT_FAILURE);
    }
    else
    {
        if(is_background)
        {
            JobControl::background_jobs.push_back(
            {
                ++JobControl::job_counter,
                {pid},
                {command},
                JobStatus::RUNNING
            });

            std::cout << "[" << JobControl::job_counter << "] " << pid << std::endl;

            return ExecResult::Continue;
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            if(WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                return ExecResult::Failed;
            }
            return ExecResult::Continue;
        }
    }
}

