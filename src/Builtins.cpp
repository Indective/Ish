#include "Builtins.hpp"
#include "ShellContext.hpp"
#include "AliasManager.hpp"

#include <vector>

ExecResult builtin_cd(const std::vector<std::string>& argv)
{
    if(argv.size() != 2)
    {
        return ExecResult::Continue;
    }

    const char* dir_name = argv[1].c_str();

    if(chdir(dir_name) == 0)
    {
        return ExecResult::Continue;
    }

    std::cerr << "cd : Directory not found : " << dir_name << std::endl;

    return ExecResult::Continue;
}

ExecResult builtin_exit(const std::vector<std::string>& argv)
{
    for(auto& job : JobControl::jobs)
    {
        if(!JobControl::is_done(job))
        {
            std::cout << "Cannot exit, Background processes running" << std::endl;
            return ExecResult::Continue;
        }
        if(!JobControl::is_stopped(job))
        {
            std::cout << "Cannot exit, Background processes stopped" << std::endl;
            return ExecResult::Continue;      
        }
    }

    return ExecResult::Exit;
}

ExecResult builtin_jobs(const std::vector<std::string>& argv)
{
    for(auto& job : JobControl::jobs)
    {
        if(!JobControl::is_done(job))
        {
            rl_on_new_line();

            std::cout << "[" << job.id << "]";

            if(JobControl::is_stopped(job))
            {
                std::cout << "\tstopped\t";
            }
            else
            {
                std::cout << "\trunning\t";
            }
            
            for(auto& command : job.commands)
            {
                for(auto& arg : command.argv)
                {
                    std::cout << arg << " ";
                }
            }

            std::cout << std::endl;
        }
    }

    return ExecResult::Continue;
}

ExecResult builtin_source(const std::vector<std::string> & argv)
{
    ShellContext shell;
    if(argv.size() != 2)
    {
        return ExecResult::Continue;
    }

    AliasManager::reload_aliases(argv[1]);
    return ExecResult::Continue;
}

ExecResult builtin_bg(std::list<JobData>::iterator job)
{
    if(job->state == JobState::STOPPED)
    {
        pid_t pgid = job->processes[0].pgid;

        kill(-pgid, SIGCONT);
    }

    job->state = JobState::RUNNING;
    job->mode = JobMode::BACKGRROUND;

    rl_on_new_line();

    std::cout << "[" << job->id << "]+ continued\t";

    for (const auto &command : job->commands)
    {
        for (const auto &arg : command.argv)
        {
            std::cout << arg << " ";
        }
    }

    std::cout << std::endl;

    return ExecResult::Continue;
}

ExecResult builtin_fg(std::list<JobData>::iterator job)
{

    return ExecResult::Continue;
}
