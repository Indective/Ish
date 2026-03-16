#include "JobControl.hpp"


namespace JobControl
{
    int job_counter = 0;
    std::vector<Job> jobs;

    bool is_background(const std::vector<std::string>& tokens)
    {
        return tokens.back() == "&";
    }
}


