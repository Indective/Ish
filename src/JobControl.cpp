#include "JobControl.hpp"
#include <string>
#include <vector>

namespace JobControl
{
    int job_counter = 0;
    std::vector<Job> jobs;

    bool handle_background(std::vector<std::string>& tokens)
    {
        if(tokens.back() == "&")
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
        // Loop because multiple children could have exited between signals
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) 
        {
            // mark the matching job as Done
            for (auto& job : jobs) 
            {
                if (job.pid == pid) 
                {
                    job.status = JobStatus::DONE;
                    break;
                }
            }
        }
    }

    void reap_finished_jobs()
    {
        for(auto &job : jobs)
        {
            if(job.status == JobStatus::DONE)
            {
                rl_on_new_line();
                std::cout << "[" << job.id << "]+" << "\tdone\t";
                for(auto &it : job.command)
                {
                    std::cout << it << " ";
                }
                std::cout << std::endl;
            }
        }
        std::erase_if(jobs, [](Job job) {return job.status == JobStatus::DONE;});
    }
}
