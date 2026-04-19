#include "Redirection.hpp"

void Redirect_stdout_overwrite(const std::string& filename)
{
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if(fd == -1)
    {
        perror("open");
        _exit(EXIT_FAILURE);
    }

    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        _exit(EXIT_FAILURE);
    }

    close(fd);
}

void Redirect_stdout_append(const std::string& filename)
{
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0664);

    if(fd == -1)
    {
        perror("open");
        _exit(EXIT_FAILURE);
    }

    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        _exit(EXIT_FAILURE);
    }

    close(fd);
}

void Redirect_stderr(const std::string& filename)
{
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if(fd == -1)
    {
        perror("open");
        _exit(EXIT_FAILURE);
    }

    if(dup2(fd, STDERR_FILENO) == -1)
    {
        perror("dup2");
        _exit(EXIT_FAILURE);
    }

    close(fd);
}

void Redirect_stdin(const std::string& filename)
{
    int fd = open(filename.c_str(), O_RDONLY);

    if(fd == -1)
    {
        perror("open");
        _exit(EXIT_FAILURE);
    }

    if(dup2(fd, STDIN_FILENO) == -1)
    {
        perror("dup2");
        _exit(EXIT_FAILURE);
    }

    close(fd);
}

void Redirect_stdin_heredoc(const std::string& delimiter)
{
    int pipefd[2];

    if(pipe(pipefd) == -1)
    {
        perror("pipe");
        _exit(EXIT_FAILURE);
    }

    std::vector<std::string> content;
    std::string line;

    std::cout << ". ";

    while(std::getline(std::cin, line) && line != delimiter)
    {
        content.push_back(line);
        std::cout << ". ";
    }

    for(auto& line : content)
    {
        write(pipefd[1], line.c_str(), line.size());
        write(pipefd[1], "\n", 1);
    }

    close(pipefd[1]);

    if(dup2(pipefd[0], STDIN_FILENO) == -1)
    {
        perror("dup2");
        _exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
}

void Redirect_stdin_herestr(const std::string& content)
{
    int pipefd[2];

    if(pipe(pipefd) == -1)
    {
        perror("pipe");
        _exit(EXIT_FAILURE);
    }

    write(pipefd[1], content.c_str(), content.size());
    write(pipefd[1], "\n", 1);

    close(pipefd[1]);

    if(dup2(pipefd[0], STDIN_FILENO) == -1)
    {
        perror("dup2");
        _exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
}