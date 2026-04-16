#include "Parsing.hpp"
#include "ShellContext.hpp"
#include <vector>
#include <sstream>
#include <algorithm> 
#include <string>

namespace CommandParsing
{
    std::optional<PipeLine> parse_command(std::string line)
    {
        ShellContext shell;
        PipeLine p;
        std::vector<std::string> tokens;
        std::string token;
        bool in_quote = false;

        if(line.back() == '&')
        {
            p.is_background = true;
            line.pop_back();
        }
        for(size_t i = 0; i < line.size(); i++)
        {
            if(line[i] == '"')
            {
                in_quote = !in_quote;
            }
            else if(line[i] == '|' && !in_quote)
            {
                p.is_pipe = true;
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
            else if(line[i] == ' ' && !in_quote)
            {
                if(!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
            }
            else if(line[i] == '&' && !in_quote)
            {
                if(i < line.size() - 1 && line[i + 1] == '&')
                {
                    if(!tokens.empty())
                    {
                        i++;
                        p.commands.push_back({tokens});
                        token.clear(); // just to be safe
                        tokens.clear();
                    }
                }
                
            }
            else
            {
                token += line[i];
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

        if(!tokens.empty())
        {
            p.commands.push_back({tokens});
        }

        for(auto &command : p.commands)
        {
            shell.replace_alias(command);
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
