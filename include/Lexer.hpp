#pragma once
#include "token.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <string_view>

class Lexer
{
private : 
    static constexpr const std::array<std::string_view, 6> redirection_ops;
public :
    std::vector<Token> tokenize(const std::string& line);
};