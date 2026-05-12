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

enum class State
{
    RUNNING,
    DONE,
    STOPPED
};

struct Process
{
    pid_t pid;
    State state;
};

struct JobData
{
    int id;
    std::vector<Process> processes;
    std::vector<Command> commands;
    bool is_done = false;
    bool is_stopped = false;
};

namespace JobControl
{
    extern volatile sig_atomic_t child_changed;
    extern volatile sig_atomic_t sigint;
    extern int job_counter;
    extern std::vector<JobData> background_jobs;
    extern std::vector<JobData> foreground_jobs;

    void reap_finished_jobs();
    void update_job_status(JobData &job, pid_t pid, int status);

    void install_sigchld();
    void install_sigint();
    void install_sigttou();
    void install_sigstp();
    void install_sigsttin();

    int rl_sigint_redisplay(void);
}