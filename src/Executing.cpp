#include "Executing.hpp"
#include "Parsing.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <fcntl.h>
#include <algorithm>
#include <iterator>

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

    ExecResult execute_foreground(const Command& cmd)
    {
        std::vector<char*> argv;
        pid_t pid = fork();
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !");
            return ExecResult::ERROR;
        }
        else if(pid == 0) // code accessible only by the child process
        {
            for(auto &arg : cmd.tokens)
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            for(auto &[op, fn] : cmd.redirect)
            {
                redircetion_handler[op](fn);
            }
            
            execvp(argv[0],argv.data());
            perror("ish");
            _exit(EXIT_FAILURE);
            
        }
        else
        {
            
            waitpid(pid,nullptr,0);
            return ExecResult::OK;
            
        }
        return ExecResult::ERROR;     
    }

    ExecResult execute_background(const Command& cmd)
    {
        std::vector<char*> argv;
        pid_t pid = fork();
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !");
            return ExecResult::ERROR;
        }
        else if(pid == 0) // code accessible only by the child process
        {
            for(auto &arg : cmd.tokens)
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            for(auto &[op, fn] : cmd.redirect)
            {
                redircetion_handler[op](fn);
            }
            
            execvp(argv[0],argv.data());
            perror("ish");
            _exit(EXIT_FAILURE);
            
        }
        else
        {
            JobControl::jobs.push_back({++JobControl::job_counter ,pid,cmd.tokens , JobStatus::RUNNING});
            std::cout << "[" << JobControl::job_counter << "] " << pid << std::endl; 
            return ExecResult::OK;
            
        }
        return ExecResult::ERROR;     
    }

    ExecResult execute_pipe(Command &cmd)
    {
        auto it = std::find(cmd.tokens.begin(), cmd.tokens.end(),"|");
        size_t pipe_index = std::distance(cmd.tokens.begin(), it);
        cmd.tokens.erase(cmd.tokens.begin() + pipe_index);
        pipe_index--;

        Command cmd1, cmd2;
        std::vector<std::string> tokens1(std::make_move_iterator(cmd.tokens.begin()), std::make_move_iterator(cmd.tokens.begin() + pipe_index));
        std::vector<std::string> tokens2(std::make_move_iterator(cmd.tokens.begin() + pipe_index),std::make_move_iterator(cmd.tokens.end()));

        cmd1.tokens = tokens1;
        cmd2.tokens = tokens2;

        // create and set up pipe
        int fd[2];
        if(pipe(fd) == -1)
        {
            perror("pipe : ");
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

    ExecResult handle_execution(Command &cmd)
    {
        if(cmd.is_pipe)
        {
            return execute_pipe(cmd);
        }
        else
        {
            if(is_builtin(cmd.tokens))
            {
                return execute_builtin(cmd);
            }

            else
            {
                if(JobControl::handle_background(cmd.tokens))
                {
                    return execute_background(cmd);
                }
                else
                {
                    return execute_foreground(cmd);
                }
            }
        } 
    }

    //Builtin implementations

    ExecResult builtin_cd(const std::vector<std::string>& tokens)
    {
        const char* dir_name;
        if (tokens.size() != 2)
        {
            return ExecResult::ERROR;
        }
        dir_name = tokens[1].c_str();

        if (chdir(dir_name) == 0)
        {
            return ExecResult::OK;
        }
        else 
        {
           std::cerr << "cd : Directory not found : " << dir_name << std::endl; 
        }
        
        return ExecResult::ERROR;
    }

    ExecResult builtin_exit(const std::vector<std::string>& tokens)
    {
        for(auto &it : JobControl::jobs)
        {
            if(it.status == JobStatus::RUNNING)
            {
                std::cout << "Cannot exit, Background proccesses running" << std::endl;
                return ExecResult::OK;
            }
        }
        return ExecResult::EXIT;
    }

    ExecResult builtin_jobs(const std::vector<std::string>& tokens)
    {
        for(auto &job : JobControl::jobs)
        {
            if(job.status == JobStatus::RUNNING)
            {
                rl_on_new_line();
                std::cout << "[" << job.id << "]+" << "\trunning\t";
                for(auto &it : job.command)
                {
                    std::cout << it << " ";
                }
                std::cout << std::endl;
            }
        }

        return ExecResult::OK;
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