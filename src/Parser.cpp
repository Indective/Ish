#include "token.hpp"
#include <vector>
#include <sstream>
#include <algorithm> 
#include <string>

namespace CommandParsing
{
    /*void handle_redirection(Command &cmd)
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
    */
}
