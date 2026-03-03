#include "Environment.hpp"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>

std::vector<std::string> Environment::parse_line(const std::string &line)
{
    // stage 1 : tokenize input
    std::vector<std::string> args;
    std::vector<std::string> result;
    std::string text;                         
    std::stringstream ss(line);            

    while (ss >> text) 
    {
        args.push_back(text);
    }
    // stage 1.5 : check command syntax
    if(!check_syntax(args))
    {
        std::cerr << "Error loading aliases, Invalid command syntax" << std::endl;
        return {};
    }

    // stage 2 : handle qoutes within second token, eg : a="cd"
    std::istringstream iss(args[1]);
    std::string unquoted_output; // end result

    // Use std::quoted during extraction
    if (iss >> std::quoted(unquoted_output)) 
    {
        std::cout << "Original: " << args[1] << std::endl; // debug
        std::cout << "Unquoted: " << unquoted_output << std::endl; // debug
        args[1] = unquoted_output;
    } 
    else 
    {
        std::cerr << "Error parsing aliases" << std::endl;
        return {}; 
    }

    std::string assignment = args[1];
    size_t pos = assignment.find("=");

    std::string name = assignment.substr(0, pos);
    std::string value = assignment.substr(pos + 1);
    result.push_back("alias");
    result.push_back(name);
    result.push_back(value);

    return result;
}

bool Environment::check_syntax(const std::vector<std::string> args)
{
    if(args.size() != 2)
    {
        return false;
    }
    std::string assignment = args[1];
    size_t pos = assignment.find("=");
    if(pos == std::string::npos)
    {
        std::cerr << "Unable to load aliases : Expected '='" << std::endl;
        return false;
    }
    std::string name  = assignment.substr(0, pos);
    std::string value = assignment.substr(pos + 1);
    if(name.empty())
    {
        std::cerr << "Unable to load alises : no alias name provided" << std::endl;
        return false;
    }

    if(value.empty())
    {
        std::cerr << "Unable to load alises : no alias value provided" << std::endl;
        return false;       
    }
    if(assignment[pos + 1] != '\"')
    {
        std::cerr << "Unable to load aliases : alias value must be quoted" << std::endl;
        return false;
    }
    if(assignment[assignment.length() - 1] != '\"')
    {
        std::cerr << "Unable to load aliases : alias value must be quoted" << std::endl;
        return false;
    }

    return true;

}

ExecResult Environment::load_aliases()
{
    std::string line;
    std::ifstream inputFile(filename);
    std::vector<std::string> args;

    if (inputFile.is_open()) 
    {
        while (std::getline(inputFile, line)) 
        {
            args = parse_line(line);
            if(args.empty())
            {
                return ExecResult::ERROR;
            }
            aliases[args[1]] = args[2];
        }
        inputFile.close();
    } 
    else 
    {
        std::cerr << "Unable to load aliases, Error opening file :  .aliases" << std::endl;
        return ExecResult::ERROR;
    }

    return ExecResult::OK;
}
