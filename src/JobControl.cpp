#include "JobControl.hpp"


namespace JobControl
{
    int job_counter = 0;
    std::vector<Job> jobs;

    bool is_background(const std::vector<std::string>& tokens)
    {
        return tokens.back() == "&";
    }
    void sigchldHandler(int)
    {
        int status;
        pid_t pid;
        // Loop because multiple children could have exited between signals
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            // mark the matching job as Done
            for (auto& job : jobs) {
                if (job.pid == pid) {
                    job.status = JobStatus::DONE;
                    break;
                }
            }
        }
    }
}
