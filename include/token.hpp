#pragma once
#include <iostream>

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