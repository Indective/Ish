#include "Parsing.hpp"
#include <vector>
#include <sstream>


namespace CommandParsing
{
    std::vector<std::string> parse_command(const std::string& line)
    {
        std::vector<std::string> tokens;
        std::string token;
        bool in_qoute = false;

        for (char c : line) 
        {
            if(c == '"')
            {
                in_qoute = !in_qoute;
            }
            else if(c == ' ' && !in_qoute)
            {
                if(!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
            }
            else
            {
                token += c;
            }
        }
        if(!token.empty()) // single command with no whitespaces entered
        {
            tokens.push_back(token);
        }
        return tokens;
    }
}
