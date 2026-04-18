#include "token.hpp"
#include "Parser.hpp"
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
        else
        {
            // must have target after redirect
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type != TokenType::Word)
            {
                std::cerr << "Parsing error: expected redirection target\n";
                return std::nullopt;
            }

            redirections.push_back(
                Redirection{
                    tokens[i].type,
                    tokens[i + 1].value
                }
            );

            ++i; // consume redirect target
        }
    }

    if (argv.empty())
    {
        std::cerr << "Parsing error: expected command\n";
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
                std::cerr << "Parsing error: expected command near '|'\n";
                return std::nullopt;
            }

            auto command = Parse_Command(current);
            if (!command)
                return std::nullopt;

            p.commands.push_back(*command);
            current.clear();
        }
        else if (tokens[i].type != TokenType::End)
        {
            current.push_back(tokens[i]);
        }
    }

    if (current.empty())
    {
        std::cerr << "Parsing error: expected command after '|'\n";
        return std::nullopt;
    }

    auto command = Parse_Command(current);
    if (!command)
        return std::nullopt;

    p.commands.push_back(*command);

    return p;
}

std::optional<AndChain> Parser::Parse_Chain(const std::vector<Token>& tokens)
{
    AndChain chain;
    std::vector<Token> current;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::Ampersand)
        {
            std::cerr << "Parsing error: '&' only allowed at end of command\n";
            return std::nullopt;
        }
        else if (tokens[i].type == TokenType::AndAnd)
        {
            if (current.empty())
            {
                std::cerr << "Parsing error: expected command near '&&'\n";
                return std::nullopt;
            }

            auto pipe = Parse_Pipeline(current);
            if (!pipe)
                return std::nullopt;

            chain.pipelines.push_back(*pipe);
            current.clear();
        }
        else if (tokens[i].type != TokenType::End)
        {
            current.push_back(tokens[i]);
        }
    }

    if (current.empty())
    {
        std::cerr << "Parsing error: expected command after '&&'\n";
        return std::nullopt;
    }

    auto pipe = Parse_Pipeline(current);
    if (!pipe)
        return std::nullopt;

    chain.pipelines.push_back(*pipe);

    return chain;
}

std::optional<Job> Parser::Parse_Job(const std::vector<Token>& tokens)
{
    if (tokens.empty())
    {
        std::cerr << "Parsing error: empty input\n";
        return std::nullopt;
    }

    Job job;
    std::vector<Token> local_tokens = tokens;

    if (!local_tokens.empty() &&
        local_tokens.back().type == TokenType::Ampersand)
    {
        job.background = true;
        local_tokens.pop_back();
    }

    auto chain = Parse_Chain(local_tokens);
    if (!chain)
        return std::nullopt;

    job.chain = *chain;

    return job;
}