#include "Executing.hpp"
#include "Parsing.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <fcntl.h>


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

    std::pair<int, int> Redirect_stdout_overwrite(const std::string& filename);
    std::pair<int, int> Redirect_stdout_append(const std::string& filename);
    std::pair<int, int> Redirect_stderr(const std::string& filename);
    std::pair<int, int> Redirect_stdin(const std::string& filename);
    std::pair<int, int> Redirect_stdin_heredoc(const std::string& filename);
    std::pair<int, int> Redirect_stdin_herestr(const std::string& filename);

    static std::unordered_map<std::string,RedirectionFn> redircetion_handler = 
    {
        {">",Redirect_stdout_overwrite},
        {">>",Redirect_stdout_append},
        {"2>",Redirect_stderr},
        {"<",Redirect_stdin},
        {"<<",Redirect_stdin_heredoc},
        {"<<<",Redirect_stdin_herestr}
    };

    std::vector<std::pair<int, int>> redirected_descriptors;

    ExecResult execute_foreground(const Command& cmd)
    {
        std::vector<char*> argv;
        pid_t pid = fork();
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !\n");
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
                redirected_descriptors.push_back(redircetion_handler[op](fn));
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
        pid_t pid;
        pid = fork();
        std::vector<char*> argv;
        if(pid == -1) // forking failed 
        {
            perror("Failed to fork process !\n");
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
                redirected_descriptors.push_back(redircetion_handler[op](fn));
            }

            execvp(argv[0],argv.data());
            perror("ish");
            _exit(EXIT_FAILURE);
            
        }
        else
        {
            JobControl::job_counter++;
            JobControl::jobs.push_back({JobControl::job_counter,pid,cmd.tokens, JobStatus::RUNNING});
            std::cout << "[" << JobControl::job_counter << "] " << pid << std::endl; 
            return ExecResult::OK;
        }
    }

    void Restore_file_descriptors()
    {
        if(!redirected_descriptors.empty())
        {
            for(auto &it : redirected_descriptors)
            {
                dup2(it.first, it.second);
                close(it.first);
            }
        }
    }

    ExecResult execute_builtin(const Command & cmd)
    {
        for(auto &[op, fn] : cmd.redirect)
        {
            redirected_descriptors.push_back(redircetion_handler[op](fn));
        }
        auto it = builtins.find(cmd.tokens[0]);
        return it->second(cmd.tokens);
    }

    bool is_builtin(const std::vector<std::string>& tokens)
    {
        return builtins.find(tokens[0]) != builtins.end();
    }

    ExecResult handle_execution(const Command &cmd)
    {
        if(CommandExecuting::is_builtin(cmd.tokens))
        {
            return CommandExecuting::execute_builtin(cmd);
        }

        else
        {
            if(cmd.is_background)
            {
                return execute_background(cmd);
            }
            
            return execute_foreground(cmd);
        }

        std::cout << "handle command error " << std::endl;
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

    std::pair<int, int> Redirect_stdout_overwrite(const std::string &filename)
    {
        int saved_out = dup(STDOUT_FILENO);
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
        return {saved_out,STDOUT_FILENO};
    }

    std::pair<int, int> Redirect_stdout_append(const std::string &filename)
    {
        int saved_out = dup(STDOUT_FILENO);
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
        return {saved_out, STDOUT_FILENO};
    }

    std::pair<int, int> Redirect_stderr(const std::string &filename)
    {
        int saved_out = dup(STDERR_FILENO);
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
        return {saved_out, STDERR_FILENO};
    }

    std::pair<int, int> Redirect_stdin(const std::string &filename)
    {
        int saved_out = dup(STDIN_FILENO);
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
        return {saved_out, STDIN_FILENO};
    }

    std::pair<int, int> Redirect_stdin_heredoc(const std::string &filename)
    {
        return {};
    }
    
    std::pair<int, int> Redirect_stdin_herestr(const std::string &filename)
    {
        int saved_out = dup(STDIN_FILENO);
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
        return {saved_out, STDIN_FILENO};
    }

}