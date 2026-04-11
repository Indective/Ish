#include "JobControl.hpp"
#include <algorithm>

namespace JobControl
{
    int job_counter = 0;
    std::vector<Job> background_jobs;

    bool handle_background(std::vector<std::string>& tokens)
    {
        if (tokens.back() == "&")
        {
            tokens.pop_back();
            return true;
        }
        return false;
    }

    void sigchldHandler(int)
    {
        int status;
        pid_t pid;

        // Reap all finished children
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            for (auto& job : background_jobs)
            {
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

    void reap_finished_jobs()
    {
        for (const auto& job : background_jobs)
        {
            if (job.status == JobStatus::DONE)
            {
                rl_on_new_line();
                std::cout << "[" << job.id << "]+" << "\tdone\t";

                for (const auto& command : job.commands)
                {
                    for(const auto &it : command.tokens)
                    {
                        std::cout << it << std::endl;
                    }
                }

                std::cout << std::endl;
            }
        }

        std::erase_if(background_jobs, [](const Job& job)
        {
            return job.status == JobStatus::DONE;
        });
    }
}