#include "Executor.hpp"
#include "Parser.hpp"
#include "Types.hpp"
#include "Redirection.hpp"
#include "Builtins.hpp"
#include "ShellContext.hpp"
#include "Signal.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <fcntl.h>
#include <algorithm>
#include <iterator>
#include <optional>
#include <signal.h>
#include <string.h>
#include <errno.h>

ExecResult Executor::execute_chain(const AndChain& chain, const bool& is_background)
{
    ExecResult result = ExecResult::Continue;

    for(size_t i = 0; i < chain.pipelines.size(); i++)
    {
        result = execute_pipe(chain.pipelines[i], is_background);
        if(result != ExecResult::Continue)
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

void Executor::restore_signal_handling()
{
    struct sigaction sa;

    // clear struct to avoid garbage values
    memset(&sa, 0, sizeof(sa));
    
    sa.sa_handler = SIG_DFL; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // restore signals that the parent changed
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);
    sigaction(SIGTTIN, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

ExecResult Executor::wait_job(std::list<JobData>::iterator &job_it, const sigset_t &oldmask)
{
    sigset_t suspend_mask = oldmask;
    sigdelset(&suspend_mask, SIGCHLD);

    std::cerr << "SHELL PGID: " << getpgrp()
            << " FG: " << tcgetpgrp(STDIN_FILENO)
            << std::endl;

    ExecResult res = ExecResult::Continue;
    
    while (!JobControl::is_stopped(*job_it) && !JobControl::is_done(*job_it))
    {
        sigsuspend(&suspend_mask);

        JobControl::reap_finished_jobs();
    }

    if(JobControl::is_stopped(*job_it))
    {
        res = ExecResult::Stopped;
    }
    else if (JobControl::is_done(*job_it))
    {
        res = ExecResult::Continue;
    }
    else if(!JobControl::succeeded(*job_it))
    {
        std::cout << "sucedada" << std::endl;
        res = ExecResult::Failed;
    }

    JobControl::handle_done_jobs();

    return res;
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

bool Executor::is_builtin(const std::vector<std::string>& argv)
{
    return builtins.find(argv[0]) != builtins.end();
}

ExecResult Executor::execute_pipe(const Pipeline& p, const bool& is_background)
{
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    pid_t pgid = 0;
    ShellContext shell;
    std::vector<std::array<int, 2>> pipes;
    ExecResult result = ExecResult::Continue;

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
        size_t cmd_count = p.commands.size();
        std::vector<Process> processes;

        processes.reserve(cmd_count);

        pipes.resize(cmd_count - 1);

        for(size_t i = 0; i < pipes.size(); i++)
        {
            if(pipe(pipes[i].data()) == -1)
            {
                perror("pipe");
                return ExecResult::Continue;
            }
        }

        // fork children
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

                restore_signal_handling();

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

                processes.push_back({pid, pgid,ProcessState::RUNNING});

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
            JobData job(++JobControl::job_counter, processes, p.commands, JobState::RUNNING, JobMode::BACKGRROUND);
            
            auto job_it = JobControl::jobs.emplace(JobControl::jobs.end(), job);

            for(auto &process : processes)
            {
                JobControl::pid_to_job[process.pid] = job_it;
            }
         
            // Print job data 
            std::cout << "[" << JobControl::job_counter << "]";

            for(auto& process : processes)
            {
                std::cout << " " << process.pid;
            }

            std::cout << std::endl;

            return result;
        }
        else
        {
            JobData job(-1, processes, p.commands, JobState::RUNNING, JobMode::FOREGROUND);

            auto job_it = JobControl::jobs.emplace(JobControl::jobs.end(), job);

            // assign all pids to the same job iterator
            for(auto &process : processes)
            {
                JobControl::pid_to_job[process.pid] = job_it;
            }

            // give control to terminal foreground group
            tcsetpgrp(STDIN_FILENO, pgid);

            ExecResult result = wait_job(job_it, oldmask);

            // hand control back to shell
            tcsetpgrp(STDIN_FILENO, shell.shell_pid);
            
            return result;
        }
    }
}




ExecResult Executor::execute_external_command(const Command& command, const bool& is_background)
{
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    std::vector<char*> argv;
    ExecResult result = ExecResult::Continue;

    pid_t pid = fork();

    if(pid == -1)
    {
        perror("fork");
        return ExecResult::Continue;
    }
    else if(pid == 0)
    {
        setpgid(0,0);

        for(auto& arg : command.argv)
        {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }

        argv.push_back(nullptr);

        for(auto& [op, fn] : command.redirections)
        {
            redircetion_handler[op](fn);
        }

        restore_signal_handling();

        execvp(argv[0], argv.data());
        perror("ish");
        _exit(EXIT_FAILURE);
    }
    else
    {
        if(is_background)
        {
            JobData job(++JobControl::job_counter, {{pid, pid,ProcessState::RUNNING}}, {command}, JobState::RUNNING, JobMode::BACKGRROUND);

            auto job_it = JobControl::jobs.emplace(JobControl::jobs.end(), job);

            JobControl::pid_to_job[pid] = job_it;

            std::cout << "[" << JobControl::job_counter << "] " << pid << std::endl;

            sigprocmask(SIG_SETMASK, &oldmask, nullptr);

            return result;
        }
        else
        {            
            ShellContext shell;
            JobData job(-1, {{pid, pid ,ProcessState::RUNNING}}, {command}, JobState::RUNNING, JobMode::FOREGROUND);
            
            auto job_it = JobControl::jobs.emplace(JobControl::jobs.end(), job);

            JobControl::pid_to_job[pid] = job_it;

            setpgid(pid, pid);
            tcsetpgrp(STDIN_FILENO, pid);     // give terminal

            result = wait_job(job_it, oldmask);

            tcsetpgrp(STDIN_FILENO, shell.shell_pid); // take terminal

            return result;
        }
    }
}

