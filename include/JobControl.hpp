#pragma once
#include "CommandModel.hpp"
#include "Executing.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sys/wait.h>
#include <readline/readline.h>
#include <csignal>
#include <signal.h>

enum class JobStatus
{
    RUNNING,
    DONE,
    STOPPED
};

struct Job
{
    int id;
    std::vector<pid_t> pids;
    std::vector<Command> commands;
    JobStatus status;
};

namespace JobControl
{
    extern int job_counter;
    extern std::vector<Job> background_jobs;

    void sigchldHandler(int);
    void reap_finished_jobs();
}