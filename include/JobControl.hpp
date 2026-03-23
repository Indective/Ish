#pragma once
#include "Command.hpp"
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
    pid_t pid;
    std::vector<std::string> command;
    JobStatus status;
};

namespace JobControl
{
    extern volatile sig_atomic_t child_changed;
    extern int job_counter;
    extern std::vector<Job> jobs;
    bool is_background(const std::vector<std::string>& tokens);
    void sigchldHandler(int);
    void reap_finished_jobs();
};
