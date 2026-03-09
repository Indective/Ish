#include "Environment.hpp"
#include "Parsing.hpp"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <fstream>
#include <string>

std::vector<std::string> Environment::parse_line(const std::string& line)
{
    size_t first_space = line.find(' ');
    if(first_space == std::string::npos)
    {
        return {};
    }

    std::string command = line.substr(0, first_space);
    std::string assignment = line.substr(first_space + 1);

    if(command != "alias")
    {
        return {};
    }

    size_t equal = assignment.find('=');
    if(equal == std::string::npos)
    {
        return {};
    }

    std::string name  = assignment.substr(0, equal);
    std::string value = assignment.substr(equal + 1);

    if(name.empty())
    {
        return {};
    }

    if(value.length() < 2 || value.front() != '"' || value.back() != '"')
    {
        return {};
    }

    value = value.substr(1, value.length() - 2);

    return {name, value};
}

ExecResult Environment::load_aliases()
{
    std::string line;
    std::ifstream inputFile(filename);
    std::vector<std::string> tokens;
    int invalid_line = 0; // get the index of the line with invalid syntax(if there are any)

    if (inputFile.is_open()) 
    {
        while (std::getline(inputFile, line)) 
        {
            invalid_line ++;
            tokens = parse_line(line);
            if(tokens.empty())
            {
                std::cerr << "Invalid alias ignored; Invalid command syntax at line : " << invalid_line << std::endl;
                continue;
            }
            aliases[tokens[0]] = tokens[1];
        }
        inputFile.close();
    } 
    else 
    {
        std::cerr << "Unable to load aliases, Error opening file :  .aliases" << std::endl;
    }

    return ExecResult::OK;
}

void Environment::replace_alias(std::vector<std::string>& tokens)
{
    std::vector<std::string> result;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        auto it = aliases.find(tokens[i]);
        if (it != aliases.end())
        {
            // expand alias value into sub-tokens and splice them in
            std::vector<std::string> expanded = CommandParsing::parse_command(it->second);

            result.insert(result.end(), expanded.begin(), expanded.end());
        }
        else
        {
            result.push_back(tokens[i]);
        }
    }
    tokens = result;
}

std::string Environment::shorten_path(const std::string &path)
{
    const char* home = getenv("HOME");
    if (home && path.rfind(home, 0) == 0) // check if path starts with $HOME
    {
        return "~" + path.substr(strlen(home)) + prefix;
    }
    return path;
}
