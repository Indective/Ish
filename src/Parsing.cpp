#include "Parsing.hpp"
#include <vector>
#include <sstream>
#include <algorithm> 

namespace CommandParsing
{
    std::vector<std::string> parse_command(const std::string& line, Command & cmd)
    {
        std::vector<std::string> tokens;
        std::string token;
        bool in_quote = false;

        for (const char c : line) 
        {
            if(c == '"')
            {
                in_quote = !in_quote;
            }
            else if(c == ' ' && !in_quote)
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
        if(!token.empty())
        {
            tokens.push_back(token);
        }
        if(in_quote)
        {
            std::cout << "Incorrect command syntax; Expected quote" << std::endl;
            return {};
        }
        handle_redirection(tokens, cmd);
        return tokens;
    }

    void handle_redirection(std::vector<std::string>& tokens, Command & cmd)
    {
        std::vector<std::string> ops = {"<","<<","<<<",">",">>","2>","|"};
        for(size_t i = 0; i < tokens.size(); i++)
        {
            auto it = std::find(ops.begin(), ops.end(), tokens[i]);
            if (it != ops.end() && i + 1 < tokens.size())
            {
                if(tokens[i] == "|")
                {
                  cmd.is_pipe = true;  
                }
                else
                {
                    cmd.redirect.push_back(std::make_pair(tokens[i], tokens[i + 1]));
                    tokens.erase(tokens.begin() + i + 1); 
                    tokens.erase(tokens.begin() + i);
                    i--; // step back since element was removed 
                }
            }
        }
    }
}
