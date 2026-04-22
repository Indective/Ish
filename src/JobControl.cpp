#include "JobControl.hpp"
#include <algorithm>

namespace JobControl
{
    int job_counter = 0;
    volatile sig_atomic_t child_changed = 0;
    std::vector<JobData> background_jobs;

    void sigchldHandler(int)
    {
        child_changed = 1;
    }

    void reap_finished_jobs()
    {
        int status;
        pid_t pid;

        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            for (size_t i = 0; i < background_jobs.size(); i++)
            {
                auto &job = background_jobs[i];

                auto it = std::find(job.pids.begin(), job.pids.end(), pid);

                if (it != job.pids.end())
                {
                    job.pids.erase(it);

                    if (job.pids.empty())
                    {
                        job.status = JobStatus::DONE;
                    }

                    break;
                }
            }
        }
    }

    void print_finished_jobs()
    {
        for (size_t i = 0; i < background_jobs.size(); )
        {
            auto &job = background_jobs[i];

            if (job.status == JobStatus::DONE)
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
}