#pragma once
#include "Types.hpp"
#include "Executor.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sys/wait.h>
#include <readline/readline.h>
#include <csignal>
#include <signal.h>

namespace JobControl
{
    extern int job_counter;
    extern std::vector<JobData> background_jobs;
    extern std::vector<JobData> foreground_jobs;

    void reap_finished_jobs();
    void update_job_status(JobData& job, pid_t pid, int status, ExecResult &result);
}