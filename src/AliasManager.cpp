#include "AliasManager.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace
{
    std::unordered_map<std::string, std::string> aliases;

    std::vector<std::string> split(const std::string & line)
    {
        std::vector<std::string> tokens;
        std::istringstream ss(line);

        std::string token;
        while(ss >> token)
        {
            tokens.push_back(token);
        }

        return tokens;
    }
}

namespace AliasManager
{
    void load_aliases()
    {
        std::string line;
        const char * home = getenv("HOME");

        std::string aliases_path = std::string(home) + filename;
        std::ifstream inputFile(aliases_path);
        int invalid_line = 0; // get the index of the line with invalid syntax(if there are any)

        if (inputFile.is_open()) 
        {
            while (std::getline(inputFile, line)) 
            {
                if(!line.empty())
                {
                    invalid_line ++;
                    
                    std::optional<std::vector<AliasToken>> tokens = tokenize_alias(line);
                    std::optional<Alias> alias = parse_alias(*tokens);
                    
                    if(alias)
                    {
                        aliases[alias->name] = alias->value;
                    }
                    else
                    {
                        std::cerr << "Could not load aliases, Incorrect syntax at line : " << invalid_line << std::endl;
                        return;
                    }
                }
            }
            inputFile.close();
        }
    }

    void replace_alias(Command &cmd)
    {
        for (size_t i = 0; i < cmd.argv.size(); i++)
        {
            auto it = aliases.find(cmd.argv[i]);

            if (it != aliases.end())
            {
                std::vector<std::string> replacement = split(it->second); 

                cmd.argv.erase(cmd.argv.begin() + i);
                cmd.argv.insert(cmd.argv.begin() + i,
                                replacement.begin(),
                                replacement.end());

                i += replacement.size() - 1;
            }
        }
    }

    void reload_aliases(const std::string &path)
    {
        aliases.clear(); // clear aliases to reload
        
        std::string line;
        std::ifstream inputFile(path);
        std::optional<std::vector<std::string>> tokens;
        int invalid_line = 0; // get the index of the line with invalid syntax(if there are any)

        if (inputFile.is_open()) 
        {
            while (std::getline(inputFile, line)) 
            {
                if(!line.empty())
                {
                    invalid_line ++;
                    std::optional<std::vector<AliasToken>> tokens = tokenize_alias(line);
                    std::optional<Alias> alias = parse_alias(*tokens);
                    
                    if(alias)
                    {
                        aliases[alias->name] = alias->value;
                    }
                    else
                    {
                        std::cerr << "Could not load aliases, Incorrect syntax at line : " << invalid_line << std::endl;
                        return;
                    }
                }
            }
            inputFile.close();
        } 
    }
    
    std::optional<std::vector<AliasToken>> tokenize_alias(const std::string &line)
    {
        std::vector<AliasToken> tokens;
        AliasToken token;

        for (size_t i = 0; i < line.size(); i++)
        {
            char c = line[i];

            if (std::isspace((unsigned char)c))
            {
                if (!token.value.empty())
                {
                    token.type = AliasTokenType::Word;
                    tokens.push_back(token);
                    token = {};
                }
                continue;
            }

            if (c == '"')
            {
                token.type = AliasTokenType::Value;

                while (++i < line.size() && line[i] != '"')
                {
                    token.value += line[i];
                }

                if (i == line.size())
                {
                    return std::nullopt;
                }

                tokens.push_back(token);
                token = {};
                continue;
            }

            if (c == '=')
            {
                if (!token.value.empty())
                {
                    token.type = AliasTokenType::Word;
                    tokens.push_back(token);
                    token = {};
                }

                tokens.push_back({AliasTokenType::EqualSign, "="});
                continue;
            }

            token.value += c;            
        }

        if (!token.value.empty())
        {
            token.type = AliasTokenType::Word;
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    std::optional<Alias> parse_alias(const std::vector<AliasToken> &tokens)
    {
        if(tokens.size() != 4)
        {
            std::cerr << "Alias loading error : Invalid Syntax" << std::endl;
            return std::nullopt; 
        }

        if(tokens[0].value != "alias")
        {
            std::cerr << "ALias loading error : expected alias keyword" << std::endl;
            return std::nullopt;
        }
        if(tokens[2].type != AliasTokenType::EqualSign)
        {
            std::cerr << "ALias loading error : expected equal sign" << std::endl;
            return std::nullopt;
        }

        return Alias{tokens[1].value, tokens[3].value};
    }
}