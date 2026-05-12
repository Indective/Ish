#include "JobControl.hpp"
#include <algorithm>
#include <unistd.h>
#include <signal.h>

namespace JobControl
{
    int job_counter = 0;
    volatile sig_atomic_t child_changed = 0;
    volatile sig_atomic_t sigint = 0;
    std::vector<JobData> background_jobs;
    std::vector<JobData> foreground_jobs;

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

    JobData* find_background_job(pid_t pid)
    {
        for(auto &job : background_jobs)
        {
            for(auto &proc : job.processes)
            {
                if(proc.pid == pid)
                {
                    return &job;
                }
            }
        }

        return nullptr;
    }

    void sigchldHandler(int)
    {
        child_changed = 1;
    }

    void sigintHandler(int)
    {
        sigint = 1;
    }

    void print_finished_jobs()
    {
        for (size_t i = 0; i < background_jobs.size(); )
        {
            auto &job = background_jobs[i];

            if (job.is_done == true)
            {
                std::cout << "[" << job.id << "]+ done\t";

                for (const auto &command : job.commands)
                {
                    for (const auto &arg : command.argv)
                    {
                        std::cout << arg << " ";
                    }
                }

                std::cout << std::endl;

                background_jobs.erase(background_jobs.begin() + i);
            }
            else
            {
                ++i;
            }
        }

        if (background_jobs.empty())
        {
            job_counter = 0;
        }
    }

    // public functions
    void reap_finished_jobs()
    {
        int status;
        pid_t pid;

        while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
        {
            JobData* job = find_background_job(pid);

            if(job)
            {
                update_job_status(*job, pid, status);
            }
        }

        print_finished_jobs();
    }

    void update_job_status(JobData& job, pid_t pid, int status)
    {
        Process* proc = find_process(pid, job);

        if(proc)
        {
            if(WIFEXITED(status) || WIFSIGNALED(status))
            {
                proc->state = State::DONE;
            }
            else if(WIFSTOPPED(status))
            {
                proc->state = State::STOPPED;
            }
            else if(WIFCONTINUED(status))
            {
                proc->state = State::RUNNING;
            }
        }

        bool is_all_done = true;
        bool is_all_stopped = true;

        for(auto& proc : job.processes)
        {
            if(proc.state != State::DONE)
            {
                is_all_done = false;
            }

            if(proc.state != State::STOPPED)
            {
                is_all_stopped = false;
            }
        }

        job.is_done = is_all_done;

        // prevent DONE jobs from also being STOPPED
        job.is_stopped = is_all_stopped && !is_all_done;
    }

    void install_sigchld()
    {
        struct sigaction sa{};
        sa.sa_handler = sigchldHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

        sigaction(SIGCHLD, &sa, nullptr);
    }

    void install_sigint()
    {
        struct sigaction sa{};
        sa.sa_handler = sigintHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGINT, &sa, nullptr);
    }

    void install_sigttou()
    {
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGTTOU, &sa, nullptr);
    }

    void install_sigstp()
    {
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGTSTP, &sa, nullptr);
    }

    void install_sigsttin()
    {
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGTTIN, &sa, nullptr);
    }

    int rl_sigint_redisplay(void)
    {
        if(JobControl::sigint)
        {
            JobControl::sigint = 0;
            rl_replace_line(" ",0);
            rl_on_new_line();
            std::cout << "\n";
            rl_redisplay();
        } 
        
        return 0;
    }
}