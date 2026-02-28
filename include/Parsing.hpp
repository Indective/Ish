#pragma once
#include "Command.hpp"
#include <string>
#include <iostream>

namespace parsing
{
    std::vector<std::string> parse_command(const std::string& line);
}
