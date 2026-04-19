#pragma once
#include "CommandModel.hpp"
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <fcntl.h>

using RedirectionFn = void(*)(const std::string&);

void Redirect_stdout_overwrite(const std::string& filename);
void Redirect_stdout_append(const std::string& filename);
void Redirect_stderr(const std::string& filename);
void Redirect_stdin(const std::string& filename);
void Redirect_stdin_heredoc(const std::string& delimiter);
void Redirect_stdin_herestr(const std::string& content);

static std::unordered_map<TokenType, RedirectionFn> redircetion_handler =
{
    {TokenType::STDOUT_OVERWRITE, Redirect_stdout_overwrite},
    {TokenType::STDOUT_APPEND, Redirect_stdout_append},
    {TokenType::STDERR, Redirect_stderr},
    {TokenType::STDIN, Redirect_stdin},
    {TokenType::STDIN_HEREDOC, Redirect_stdin_heredoc},
    {TokenType::STDIN_HERESTRING, Redirect_stdin_herestr}
};
