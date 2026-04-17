#include "Lexer.hpp"
#include <iostream>

const std::array<std::string_view, 6> Lexer::redirection_ops = {"<","<<","<<<",">",">>","2>"};

std::vector<Token> Lexer::tokenize(const std::string &line)
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
                throw std::runtime_error("Tokenizer : Invalid command syntx; Unclosed quote.");
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

    tokens.push_back({TokenType::End});
    return tokens;
    
}
