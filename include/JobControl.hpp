#pragma once
#include "CommandModel.hpp"
#include "Executor.hpp"
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

struct JobData
{
    int id;
    std::vector<pid_t> pids;
    std::vector<Command> commands;
    JobStatus status;
};

namespace JobControl
{
    extern volatile sig_atomic_t child_changed;
    extern volatile sig_atomic_t sigint;
    extern int job_counter;
    extern std::vector<JobData> background_jobs;

    void reap_finished_jobs();
    void print_finished_jobs();

    void sigintHandler(int);
    void sigchldHandler(int);

    void install_sigchld();
    void install_sigint();
}