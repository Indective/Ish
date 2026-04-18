#include "Lexer.hpp"
#include <iostream>
#include <array>
#include <algorithm>

const std::map<std::string, TokenType> Lexer::redirection_ops = 
{
    {">",TokenType::STDOUT_OVERWRITE},
    {">>",TokenType::STDOUT_APPEND},
    {"2>",TokenType::STDERR},
    {"<",TokenType::STDIN},
    {"<<",TokenType::STDIN_HEREDOC},
    {"<<<",TokenType::STDIN_HERESTRING}
};

std::optional<std::vector<Token>> Lexer::tokenize(const std::string &line)
{
    std::vector<Token> tokens;
    Token token;

    for (size_t i = 0; i < line.size(); i++)
    {
        char c = line[i];

        if (std::isspace((unsigned char)c))
        {
            if (!token.value.empty())
            {
                token.type = TokenType::Word;
                tokens.push_back(token);
                token = {};
            }
            continue;
        }

        if (c == '"')
        {
            token.type = TokenType::Word;

            while (++i < line.size() && line[i] != '"')
            {
                token.value += line[i];
            }

            if (i == line.size())
            {
                return std::nullopt;
            }

            tokens.push_back(token);
            token = {};
            continue;
        }

        if (c == '|')
        {
            if (!token.value.empty())
            {
                token.type = TokenType::Word;
                tokens.push_back(token);
                token = {};
            }

            tokens.push_back({TokenType::Pipe, "|"});
            continue;
        }

        if (c == '&')
        {
            if (!token.value.empty())
            {
                token.type = TokenType::Word;
                tokens.push_back(token);
                token = {};
            }

            if (i + 1 < line.size() && line[i + 1] == '&')
            {
                tokens.push_back({TokenType::AndAnd, "&&"});
                i++;
            }
            else
            {
                tokens.push_back({TokenType::Ampersand, "&"});
            }

            continue;
        }

        token.value += c;
    }

    if (!token.value.empty())
    {
        token.type = TokenType::Word;
        tokens.push_back(token);
    }
    
    for(size_t i = 0; i < tokens.size(); i++)
    {
        const auto it = redirection_ops.find(tokens[i].value);
        if (it != redirection_ops.end())
        {
            tokens[i].type = it->second;
        }
    }

    return tokens;
    
}
