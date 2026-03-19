#include "JobControl.hpp"


namespace JobControl
{
    int job_counter = 0;
    std::vector<Job> jobs;
    volatile sig_atomic_t childchanged = 0;

    bool is_background(const std::vector<std::string>& tokens)
    {
        return tokens.back() == "&";
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
        childchanged = 1;
    }

    void print_done_message_and_reap()
    {

        for(auto &job : jobs)
        {
            if(job.status == JobStatus::DONE)
            {
                rl_on_new_line();
                std::cout << "[" << job.id << "]" << " done ";
                for(auto &it : job.command)
                {
                    std::cout << it << " ";
                }
                std::cout << std::endl;
                std::cout.flush();

                rl_redisplay();
            }
        }
        std::erase_if(jobs, [](Job job) {return job.status == JobStatus::DONE;});
    }

}
