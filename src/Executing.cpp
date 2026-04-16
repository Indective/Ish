#include "Executing.hpp"
#include "Parsing.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <fcntl.h>
#include <algorithm>
#include <iterator>
#include <optional>

namespace CommandExecuting
{
    ExecResult builtin_cd(const std::vector<std::string>& tokens);
    ExecResult builtin_exit(const std::vector<std::string>& tokens);
    ExecResult builtin_jobs(const std::vector<std::string>& tokens);

    static std::unordered_map<std::string, BuiltinFn> builtins =
    {
        {"cd", builtin_cd},
        {"exit", builtin_exit},
        {"jobs",builtin_jobs}
    };

    void Redirect_stdout_overwrite(const std::string& filename);
    void Redirect_stdout_append(const std::string& filename);
    void Redirect_stderr(const std::string& filename);
    void Redirect_stdin(const std::string& filename);
    void Redirect_stdin_heredoc(const std::string& delimiter);
    void Redirect_stdin_herestr(const std::string& content);

    static std::unordered_map<std::string,RedirectionFn> redircetion_handler = 
    {
        {">",Redirect_stdout_overwrite},
        {">>",Redirect_stdout_append},
        {"2>",Redirect_stderr},
        {"<",Redirect_stdin},
        {"<<",Redirect_stdin_heredoc},
        {"<<<",Redirect_stdin_herestr}
    };

    ExecResult execute_pipe(std::optional<PipeLine>& p)
    {
        size_t cmd_count = p->commands.size();
        pid_t pgid = 0;
        std::vector<pid_t> pids;
        pids.reserve(cmd_count);

        p->pipes.resize(cmd_count - 1);

        for (size_t i = 0; i < p->pipes.size(); i++)
        {
            if (pipe(p->pipes[i].data()) == -1)
            {
                perror("pipe");
                return ExecResult::Continue;
            }
        }

        for (size_t i = 0; i < cmd_count; i++)
        {
            pid_t pid = fork();
            if (pid == -1) 
            { 
                perror("fork"); 
                return ExecResult::Continue; 
            }
            else if (pid == 0) // child
            {
                // connect stdin
                if (i > 0)
                {
                    dup2(p->pipes[i - 1][0], STDIN_FILENO);
                }

                // connect stdout
                if (i < cmd_count - 1)
                {
                    dup2(p->pipes[i][1], STDOUT_FILENO);
                }

                for (auto &[op, fn] : p->commands[i].redirect)
                {
                    redircetion_handler[op](fn);
                }

                close_pipes(p);

                std::vector<char*> argv;
                for (auto &arg : p->commands[i].tokens) 
                {
                    argv.push_back(const_cast<char*>(arg.c_str()));
                }

                argv.push_back(nullptr);
                execvp(argv[0], argv.data());
                perror("execvp : ");
                _exit(EXIT_FAILURE);
            }
            else // parent
            {
                if (i == 0) 
                {
                    pgid = pid;
                }
                if(setpgid(pid, pgid) == -1)
                {
                    perror("setpgid parent : ");
                    return ExecResult::Continue;
                }
                pids.push_back(pid);

                if (i > 0) 
                {
                    close(p->pipes[i - 1][0]);
                }
                if (i < cmd_count - 1)
                {
                    close(p->pipes[i][1]);
                }
            }
        }

        if (p->is_background)
        {   
            JobControl::background_jobs.push_back({++JobControl::job_counter, pids, p->commands, JobStatus::RUNNING});
            std::cout << "[" << JobControl::job_counter << "]";
            for (auto &pid : pids) 
            {
                std::cout << " " << pid;
            }
            std::cout << std::endl;
            return ExecResult::Continue;
        }
        else
        {
            for (pid_t pid : pids) 
            {
                waitpid(pid, nullptr, 0);
            }
            return ExecResult::Continue;
        }
        return ExecResult::Continue;
    }

    ExecResult execute_external(const Command &command, const bool &is_background)
    {
        std::vector<char*> argv;
        pid_t pid = fork();
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !");
            return ExecResult::Continue;
        }
        else if(pid == 0) // code accessible only by the child process
        {
            for(auto &arg : command.tokens)
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            for(auto &[op, fn] : command.redirect)
            {
                redircetion_handler[op](fn);
            }
            
            execvp(argv[0],argv.data());
            perror("ish");
            _exit(EXIT_FAILURE);
            
        }
        else
        {
            if (is_background)
            {
                JobControl::background_jobs.push_back({++JobControl::job_counter,{pid},{command},JobStatus::RUNNING});
                std::cout << "[" << JobControl::job_counter << "] " << pid << std::endl; 
                return ExecResult::Continue;
            }
            else
            {
                waitpid(pid,nullptr,0);
                return ExecResult::Continue;
            }            
        }
        return ExecResult::Continue;   
    }

    ExecResult handle_execution(std::optional<PipeLine> &p)
    {
        if (!p->is_pipe)
        {
            for(auto &command : p->commands)
            {
                ExecResult result;
                if(!is_builtin(command.tokens))
                {
                    result = execute_external(command, p->is_background);
                }
                else
                {
                    result = execute_builtin(command);
                }
                if(result == ExecResult::Exit)
                {
                    return result;
                }
            }
        }
        else
        {
            return execute_pipe(p);
        }
        return ExecResult::Continue;
    }

    void close_pipes(std::optional<PipeLine> &p)
    {
        for(auto &it : p->pipes)
        {
            close(it[0]);
            close(it[1]);
        }
    }

    ExecResult execute_builtin(const Command & cmd)
    {
        for(auto &[op, fn] : cmd.redirect)
        {
            redircetion_handler[op](fn);
        }
        auto it = builtins.find(cmd.tokens[0]);
        return it->second(cmd.tokens);
    }

    bool is_builtin(const std::vector<std::string>& tokens)
    {
        return builtins.find(tokens[0]) != builtins.end();
    }

    //Builtin implementations

    ExecResult builtin_cd(const std::vector<std::string>& tokens)
    {
        const char* dir_name;
        if (tokens.size() != 2)
        {
            return ExecResult::Continue;
        }
        dir_name = tokens[1].c_str();

        if (chdir(dir_name) == 0)
        {
            return ExecResult::Continue;
        }
        else 
        {
           std::cerr << "cd : Directory not found : " << dir_name << std::endl; 
        }
        
        return ExecResult::Continue;
    }

    ExecResult builtin_exit(const std::vector<std::string>& tokens)
    {
        for(auto &it : JobControl::background_jobs)
        {
            if(it.status == JobStatus::RUNNING)
            {
                std::cout << "Cannot exit, Background proccesses running" << std::endl;
            }
        }
        return ExecResult::Exit;
    }

    ExecResult builtin_jobs(const std::vector<std::string>& tokens)
    {
        for(auto &job : JobControl::background_jobs)
        {
            if(job.status == JobStatus::RUNNING)
            {
                rl_on_new_line();
                std::cout << "[" << job.id << "]+" << "\trunning\t";
                for(auto &command : job.commands)
                {
                    for(auto &token : command.tokens)
                    {
                        std::cout << token << std::endl;
                    }
                }
                std::cout << std::endl;
            }
        }

        return ExecResult::Continue;
    }

    // redircetion_handler implementations

    void Redirect_stdout_overwrite(const std::string &filename)
    {
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC , 0664);
        if(fd == -1)
        {
            perror("open ");
            _exit(EXIT_FAILURE);
        }
        if(dup2(fd,STDOUT_FILENO) == -1)
        {
            perror("dup2 ");
            _exit(EXIT_FAILURE);
        }
    }

    void Redirect_stdout_append(const std::string &filename)
    {
        int fd = open(filename.c_str(), O_APPEND | O_CREAT | O_WRONLY , 0664);
        if(fd == -1)
        {
            perror("open ");
            _exit(EXIT_FAILURE);
        }
        if(dup2(fd,STDOUT_FILENO) == -1)
        {
            perror("dup2 ");
            _exit(EXIT_FAILURE);    
        }
    }

    void Redirect_stderr(const std::string &filename)
    {
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0664);
        if(fd == -1)
        {
            perror("open ");
            _exit(EXIT_FAILURE);
        }
        if(dup2(fd,STDERR_FILENO) == -1)
        {
            perror("dup2 ");
            _exit(EXIT_FAILURE); 
        }
    }

    void Redirect_stdin(const std::string &filename)
    {
        int fd = open(filename.c_str(), O_RDONLY | O_CREAT, 0664);
        if(fd == -1)
        {
            perror("open ");
            _exit(EXIT_FAILURE);
        }
        if(dup2(fd,STDIN_FILENO) == -1)
        {
            perror("dup2 ");
            _exit(EXIT_FAILURE); 
        }
    }

    void Redirect_stdin_heredoc(const std::string &delimiter)
    {
        int pipefd[2];

        std::vector<std::string> content;
        std::string line;
        std::cout << ". ";
        while (std::getline(std::cin, line) && line != delimiter) 
        {
            content.push_back(line);
            std::cout << ". ";
        }
        if(pipe(pipefd) == -1)
        {
            perror("pipe : ");
            _exit(EXIT_FAILURE);
        }
        for(auto &line : content)
        {
            write(pipefd[1],line.c_str(),line.size());
            write(pipefd[1], "\n", 1);
        }
        close(pipefd[1]);
        if(dup2(pipefd[0],STDIN_FILENO) == -1)
        {
            perror("dup2 : ");
            _exit(EXIT_FAILURE);
        }
        close(pipefd[0]);
        
    }
    
    void Redirect_stdin_herestr(const std::string &content)
    {
        int pipefd[2];
        if(pipe(pipefd) == -1)
        {
            perror("pipe : ");
            _exit(EXIT_FAILURE);
        }
        write(pipefd[1],content.c_str(),content.size());
        write(pipefd[1], "\n", 1);
        close(pipefd[1]);
        if(dup2(pipefd[0],STDIN_FILENO) == -1)
        {
            perror("dup2 : ");
            _exit(EXIT_FAILURE);
        }
        close(pipefd[0]);
    }

}