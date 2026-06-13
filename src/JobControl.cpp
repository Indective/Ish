#include "JobControl.hpp"
#include "Signal.hpp"
#include "Executor.hpp"
#include "ShellContext.hpp"

#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

namespace JobControl
{
    int job_counter = 0;
    std::list<JobData> jobs;
    std::unordered_map<pid_t , std::list<JobData>::iterator> pid_to_job;

    // helper/ private functions
    Process* find_process(pid_t pid, JobData &job)
    {
        for(auto &p : job.processes)
        {
            if(p.pid == pid)
            {
                return &p;
            }
        }

        return nullptr;
    }

    JobState compute_job_state(JobData &job)
    {
        ShellContext shell;
        bool all_done = true;
        bool any_running = false;
        bool any_stopped = false;

        for (auto& p : job.processes)
        {
            if (p.state != ProcessState::DONE)
            {
                all_done = false;
            }

            if (p.state == ProcessState::RUNNING)
            {
                any_running = true;
            }

            if (p.state == ProcessState::STOPPED)
            {
                any_stopped = true;
            }
        }

        if (all_done)
        {
            return JobState::DONE;
        }

        if (any_stopped && !any_running)
        {
            tcsetpgrp(STDIN_FILENO, shell.shell_pid);
            return JobState::STOPPED;
        }

        return JobState::RUNNING;
    }

    bool is_stopped(const JobData &job)
    {
        return (job.state == JobState::STOPPED);
    }

    bool is_done(const JobData &job)
    {
        return (job.state == JobState::DONE);
    }
    
    bool is_background(const JobData &job)
    {
        return job.mode == JobMode::BACKGRROUND;
    }
    
    bool is_foreground(const JobData &job)
    {
        return job.mode == JobMode::FOREGROUND;
    }

    bool succeeded(const JobData &job)
    {
        return job.exit_code == 0;
    }

    void notif_stopped_job(const JobData &job)
    {
        rl_on_new_line();

        std::cout << "[" << job.id << "]+ stopped\t";

        for (const auto &command : job.commands)
        {
            for (const auto &arg : command.argv)
            {
                std::cout << arg << " ";
            }
        }

        std::cout << std::endl;
    }
    
    void notif_done_job(const JobData &job)
    {
        rl_on_new_line();

        std::cout << "[" << job.id << "]+ done\t";

        for (const auto &command : job.commands)
        {
            for (const auto &arg : command.argv)
            {
                std::cout << arg << " ";
            }
        }

        std::cout << std::endl;
    }

    // public functions

    void reap_finished_jobs()
    {
        int status;
        pid_t pid;

        while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
        {
            std::cerr << "got pid = " << pid << std::endl;

            auto it = pid_to_job.find(pid);

            if(it == pid_to_job.end())
            {
                continue;
            }

            auto job_it = it->second;

            update_job_status(*job_it, pid, status);

            JobState new_state = compute_job_state(*job_it);
            job_it->state = new_state;
        }
        
    }

    void handle_done_jobs()
    {
        ShellContext shell;

        for(auto job = jobs.begin(); job != jobs.end();)
        {
            std::cout << "iteration" << std::endl;
            if(is_done(*job))
            {
                if(is_background(*job))
                {
                    notif_done_job(*job);
                }

                job = jobs.erase(job);
            }
            else if(is_stopped(*job))
            {
                std::cout << "job is stopped" << std::endl;
                if(job->id == -1)
                {
                    std::cout << "incrementing" << std::endl;
                    job->id = job_counter++;
                }

                std::cout << "returning control to shell" << std::endl;

                std::cout << "shell owner  : " << tcgetpgrp(STDIN_FILENO) << std::endl;

                std::cout << std::endl;
                tcsetpgrp(STDIN_FILENO, shell.shell_pid);

                ++job;
            }
        }
    }

    void update_job_status(JobData& job, pid_t pid, int status)
    {
        int exit_code = 0;

        Process* proc = find_process(pid, job);

        if (!proc) 
        {
            return;
        }

        if (WIFEXITED(status))
        {
            proc->state = ProcessState::DONE;
            exit_code = WEXITSTATUS(status);
        }

        else if (WIFSIGNALED(status))
        {
            proc->state = ProcessState::DONE;
            exit_code = 128 + WTERMSIG(status);
        }

        else if (WIFSTOPPED(status))
        {
            proc->state = ProcessState::STOPPED;
        }

        else if (WIFCONTINUED(status))
        {
            proc->state = ProcessState::RUNNING;
        }

        job.exit_code = exit_code;
        std::cout << "job exit code : " << job.exit_code << std::endl;
    }
    
}