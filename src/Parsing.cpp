#include "Parsing.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

std::vector<std::string> CommandParsing::parse_command(const std::string& line)
{
    std::vector<std::string> args;
    std::string arg;                         
    std::stringstream ss(line);            

    while (ss >> arg) 
    {
        args.push_back(arg);
    }
    return args;
}
