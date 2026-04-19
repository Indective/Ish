#include "ShellContext.hpp"
#include "Parser.hpp"
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <optional>

std::unordered_map<std::string, std::string> ShellContext::aliases;

std::optional<std::vector<std::string>> ShellContext::parse_line(const std::string& line, const bool& is_alias_line)
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

    if(is_alias_line)
    {
        
        if(check_syntax(tokens, in_quote, line.find('"') != std::string::npos, line))
        {
            return tokens;
        }
        else 
        {
            return std::nullopt;
        }
    }
    else
    {
        return tokens;
    }
}

bool ShellContext::check_syntax(const std::vector<std::string> &tokens, const bool &in_quote, const bool &found_quote, const std::string& line)
{
    if(!(tokens[0] == "alias"))
    {
        std::cerr << "Incorrect command syntax, Expected 'alias'" << std::endl;
        return false;
    }
    else if(!(tokens[2] == "="))
    {
        std::cerr << "Incorrect command syntax, Expected '=' after variable name : (" << tokens[1] << ")" << std::endl;
        return false;
    }
    if(in_quote || !found_quote || line.back() != '"')
    {
        std::cout << "Incorrect alias syntax; Expected quote" << std::endl;
        return false;
    }
    else if(tokens.size() != 4)
    {
        std::cerr << "Incorrect command syntax, too many arguments" << std::endl;
        return false;
    }
    return true;
}

void ShellContext::load_aliases()
{
    std::string line;
    std::ifstream inputFile(filename);
    std::optional<std::vector<std::string>> tokens;
    int invalid_line = 0; // get the index of the line with invalid syntax(if there are any)

    if (inputFile.is_open()) 
    {
        while (std::getline(inputFile, line)) 
        {
            if(!line.empty())
            {
                invalid_line ++;
                tokens = parse_line(line, true);
                if(tokens)
                {
                    aliases[(*tokens)[1]] = (*tokens)[3];
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
    else 
    {
        std::cerr << "Unable to load aliases, Error opening file :  .aliases" << std::endl;
    }

}

void ShellContext::replace_alias(Command &cmd)
{
    for (size_t i = 0; i < cmd.argv.size(); i++)
    {
        auto it = aliases.find(cmd.argv[i]);
        if (it != aliases.end())
        {
            cmd.argv.erase(cmd.argv.begin() + i);
            std::optional<std::vector<std::string>> alias_command = parse_line(it->second, false);
            cmd.argv.insert(cmd.argv.begin() + i, alias_command->begin(), alias_command->end());
            i += alias_command->size();
        }
    }
}

std::string ShellContext::shorten_path(const std::string &path)
{
    const char* home = getenv("HOME");
    if (home && path.rfind(home, 0) == 0) // check if path starts with $HOME
    {
        return "~" + path.substr(strlen(home));
    }
    return path;
}

std::string ShellContext::build_prompt(const std::string &path)
{
    std::string short_path = shorten_path(path);
    std::string prompt = blue + short_path + reset + postfix;
    return prompt; 
}

const char * ShellContext::get_input(const char * path)
{
    char * input;
    while(true)
    {
        input = readline(path);
        if(!input)
        {
            break;
        }
        else if(input[0] != '\0')
        {
            add_history(input);
            return input;
        }
        free(input);
    }
    free(input);
    input = nullptr;
    return input;
}
