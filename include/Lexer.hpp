#pragma once
#include "token.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <string_view>
#include <map>

class Lexer
{
private : 
    static const std::map<std::string, TokenType> redirection_ops;
public :
    std::vector<Token> tokenize(const std::string& line);
};