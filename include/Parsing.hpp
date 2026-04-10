#pragma once
#include "Command.hpp"
#include "JobControl.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <optional>

namespace CommandParsing
{
    std::optional<PipeLine> parse_command(const std::string& line);
    void handle_redirection(std::vector<std::string>& tokens, Command & cmd);
}
