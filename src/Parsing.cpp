#include "Parsing.hpp"
#include <vector>
#include <sstream>
#include <algorithm> 

// to do : 
// check for redirections, if found place them in pairs in a vector, each operator corresponding to its own file
namespace CommandParsing
{
    std::vector<std::string> parse_command(const std::string& line, Command & cmd)
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
        handle_redirection(tokens, cmd);
        return tokens;
    }

    void handle_redirection(std::vector<std::string>& tokens, Command & cmd)
    {
        std::vector<std::string> ops = {"<","<<","<<<",">",">>","2>"};
        for(size_t i = 0; i < tokens.size(); i++)
        {
            auto it = std::find(ops.begin(), ops.end(), tokens[i]);
            if (it != ops.end() && i + 1 < tokens.size())
            {
                cmd.redirect.push_back(std::make_pair(tokens[i], tokens[i + 1]));
                tokens.erase(tokens.begin() + i + 1); 
                tokens.erase(tokens.begin() + i);
                i--; // step back since element was removed 
            }
        }
    }
}
