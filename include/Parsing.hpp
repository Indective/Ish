#pragma once
#include "Command.hpp"
#include "JobControl.hpp"
#include <string>
#include <iostream>

namespace CommandParsing
{
    std::vector<std::string> parse_command(const std::string& line);
    std::vector<std::string> remove_background_symbol(std::vector<std::string> &tokens);
}
