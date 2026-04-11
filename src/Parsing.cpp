#include "Parsing.hpp"
#include <vector>
#include <sstream>
#include <algorithm> 

namespace CommandParsing
{
    std::optional<PipeLine> parse_command(const std::string &line)
    {
        PipeLine p;
        std::vector<std::string> tokens;
        std::string token;
        bool in_quote = false;

        for(const char c : line)
        {
            if(c == '"')
            {
                in_quote = !in_quote;
            }
            else if(c == '|' && !in_quote)
            {
                if(token.empty() && tokens.empty())
                {
                    std::cerr << "Error: incorrect command syntax; expected command before |" << std::endl;
                    return std::nullopt;
                }
                if (!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
                p.commands.push_back({tokens});
                tokens.clear();
            }
            else if(c == '&' && !in_quote)
            {
                p.is_background = true;
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

        //check for incorrect syntax
        if(in_quote)
        {
            std::cerr << "Error: incorrect command syntax; expected closing quote" << std::endl;
            return std::nullopt;
        }
        if(line.back() == '|')
        {
            std::cerr << "Error: incorrect command syntax; expected command after |" << std::endl;
           return std::nullopt; 
        }
        if(!token.empty())
        {
            tokens.push_back(token);
        }

        // add last token
        if(!tokens.empty())
        {
            p.commands.push_back({tokens});
        }

        for(auto &command : p.commands)
        {
            handle_redirection(command);
        }
        return p;
    }

    void handle_redirection(Command &cmd)
    {
        std::vector<std::string> ops = {"<","<<","<<<",">",">>","2>","|"};
        for(size_t i = 0; i < cmd.tokens.size(); i++)
        {
            auto it = std::find(ops.begin(), ops.end(), cmd.tokens[i]);
            if (it != ops.end() && i + 1 < cmd.tokens.size())
            {
                cmd.redirect.push_back(std::make_pair(cmd.tokens[i], cmd.tokens[i + 1]));
                cmd.tokens.erase(cmd.tokens.begin() + i + 1); 
                cmd.tokens.erase(cmd.tokens.begin() + i);
                i--; // step back since element was removed    
            }
        }
    }
}
