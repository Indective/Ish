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
#include <unordered_map>
#include <list>

namespace JobControl
{
    extern int job_counter;
    extern std::list<JobData> jobs;
    extern std::unordered_map<pid_t , std::list<JobData>::iterator> pid_to_job;

    void reap_finished_jobs();
    void handle_done_jobs();
    void update_job_status(JobData& job, pid_t pid, int status);

    // some helper functions
    JobState compute_job_state(JobData &job);

    bool is_stopped(const JobData & job);
    bool is_done(const JobData & job);

    bool is_background(const JobData & job);
    bool is_foreground(const JobData & job);

    bool succeeded(const JobData & job);

    void notif_stopped_job(const JobData &job);
    void notif_done_job(const JobData &job);

}