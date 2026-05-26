#include "JobControl.hpp"
#include "Signal.hpp"
#include "Executor.hpp"

#include <algorithm>
#include <unistd.h>
#include <signal.h>

namespace JobControl
{
    int job_counter = 0;
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
                ExecResult result = ExecResult::Continue;
                update_job_status(*job, pid, status, result);
            }
        }

        print_finished_jobs();
    }

    void update_job_status(JobData& job, pid_t pid, int status, ExecResult &result)
    {
        Process* proc = find_process(pid, job);

        if(proc)
        {
            if(WIFEXITED(status))
            {
                int code = WEXITSTATUS(status);

                if(code == 0)
                {
                    proc->state = State::DONE;
                }
                else
                {
                    proc->state = State::FAILED;
                    result = ExecResult::Failed;
                }
                
            }
            else if(WIFSIGNALED(status))
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

}