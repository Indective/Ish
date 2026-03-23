#pragma once
#include "Command.hpp"
#include "JobControl.hpp"
#include <string>
#include <iostream>
#include <vector>

namespace CommandParsing
{
    std::vector<std::string> parse_command(const std::string& line);
}
