#include "token.hpp"
#include "Parser.hpp"
#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

std::optional<Command> Parser::Parse_Command(const std::vector<Token>& tokens)
{
    std::vector<std::string> argv;
    std::vector<Redirection> redirections;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::Word)
        {
            argv.push_back(tokens[i].value);
        }
        else if (is_redirection_token(tokens[i]))
        {
            if (i + 1 >= tokens.size() || tokens[i + 1].type != TokenType::Word)
            {
                std::cerr << "Parsing error: expected redirection target" << std::endl;;
                return std::nullopt;
            }

            redirections.push_back({tokens[i].type, tokens[i + 1].value});

            ++i;
        }
        else
        {
            std::cerr << "Parsing error: invalid token in command context" << std::endl;
            return std::nullopt;
        }
    }

    if (argv.empty())
    {
        std::cerr << "Parsing error: expected command" << std::endl;
        return std::nullopt;
    }

    return Command{argv, redirections};
}

std::optional<Pipeline> Parser::Parse_Pipeline(const std::vector<Token> &tokens)
{
    Pipeline p;
    std::vector<Token> current;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::Pipe)
        {
            if (current.empty())
            {
                std::cerr << "Parsing error: expected command before '|'" << std::endl;
                return std::nullopt;
            }

            auto command = Parse_Command(current);
            if (!command)
                return std::nullopt;

            p.commands.push_back(*command);
            current.clear();
        }
        else
        {
            current.push_back(tokens[i]);
        }
    }

    if (current.empty())
    {
        std::cerr << "Parsing error: expected command after '|'" << std::endl;
        return std::nullopt;
    }

    auto command = Parse_Command(current);
    if (!command)
    {
        return std::nullopt;
    }

    p.commands.push_back(*command);

    return p;
}

std::optional<AndChain> Parser::Parse_Chain(const std::vector<Token>& tokens)
{
    AndChain chain;
    std::vector<Token> current;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::AndAnd)
        {
            if (current.empty())
            {
                std::cerr << "Parsing error: expected command before '&&' " << std::endl;
                return std::nullopt;
            }

            auto pipe = Parse_Pipeline(current);
            if (!pipe)
                return std::nullopt;

            chain.pipelines.push_back(*pipe);
            current.clear();
        }
        else
        {
            current.push_back(tokens[i]);
        }
    }

    if (current.empty())
    {
        std::cerr << "Parsing error: expected command after '&&' " << std::endl;
        return std::nullopt;
    }

    auto pipe = Parse_Pipeline(current);
    if (!pipe)
    {
        return std::nullopt;
    }

    chain.pipelines.push_back(*pipe);

    return chain;
}

bool Parser::is_redirection_token(const Token &token)
{
    return  (token.type == TokenType::STDOUT_APPEND ||
            token.type == TokenType::STDOUT_OVERWRITE ||
            token.type == TokenType::STDIN ||
            token.type == TokenType::STDIN_HEREDOC ||
            token.type == TokenType::STDIN_HERESTRING ||
            token.type == TokenType::STDERR);
}

std::optional<Job> Parser::Parse_Job(const std::vector<Token>& tokens)
{
    if (tokens.empty())
    {
        std::cerr << "Parsing error: empty input" << std::endl;
        return std::nullopt;
    }

    Job job;
    std::vector<Token> local_tokens = tokens;

    if (local_tokens.back().type == TokenType::Ampersand)
    {
        job.background = true;
        local_tokens.pop_back();
    }

    auto chain = Parse_Chain(local_tokens);
    if (!chain)
    {
        return std::nullopt;
    }

    job.chain = *chain;

    return job;
}