#pragma once
#include "Command.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sys/wait.h>

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
    extern int job_counter;
    extern std::vector<Job> jobs;
    bool is_background(const std::vector<std::string>& tokens);
    void sigchldHandler(int);
};
