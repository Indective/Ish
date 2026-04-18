#include <vector>
#include <iostream>
#include <string>
#include <iostream>
#include <vector>
#include <optional>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <array>
#include <map>

enum class TokenType {
    Word,
    Pipe,
    AndAnd,
    Ampersand,
    STDOUT_APPEND, // >>
    STDOUT_OVERWRITE, // >
    STDIN_HEREDOC, // << 
    STDIN_HERESTRING,// <<<
    STDERR, // 2>
    STDIN, // < 
    End
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType type, std::string value)
    {
        this->type = type;
        this->value = value;
    }
    Token(TokenType type)
    {
        this->type = type;
        this->value = "";
    }
    Token()
    {
        this->value = "";
    }
};

namespace CommandParsing
{
    const std::map<std::string, TokenType> redirection_ops = 
    {
        {">",TokenType::STDOUT_OVERWRITE},
        {">>",TokenType::STDOUT_APPEND},
        {"2>",TokenType::STDERR},
        {"<",TokenType::STDIN},
        {"<<",TokenType::STDIN_HEREDOC},
        {"<<<",TokenType::STDIN_HERESTRING}
    };
    std::vector<Token> tokenize(const std::string& line)
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
        for(size_t i = 0; i < tokens.size(); i++)
        {
            const auto it = redirection_ops.find(tokens[i].value);
            if (it != redirection_ops.end())
            {
                tokens[i].type = it->second;
            }
        }
        tokens.push_back({TokenType::End});
        return tokens;
    }
}

int main()
{
    std::string line = "ls > test.txt";
    std::vector<Token> tokens;
    try
    {
        tokens = CommandParsing::tokenize(line);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    std::cout << line << std::endl;
    for(auto &it : tokens)
    {
        if(it.type == TokenType::Word)
        {
            std::cout << "word " << it.value << std::endl;
        }
        if(it.type == TokenType::Pipe)
        {
            std::cout << "pipe " << it.value << std::endl;
        }
        if(it.type == TokenType::End)
        {
            std::cout << "end " << it.value << std::endl;
        }
        if(it.type == TokenType::Ampersand)
        {
            std::cout << "ampersand " << it.value << std::endl;
        }
        if(it.type == TokenType::AndAnd)
        {
            std::cout << "andand " << it.value << std::endl;
        }
        if(it.type == TokenType::STDOUT_OVERWRITE)
        {
            std::cout << "overwrite " << it.value << std::endl;
        }
    }
    return 0;
}


