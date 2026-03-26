#include "Command.hpp"
#include <unordered_map>
#include <string>

class Environment
{
private:
    // variables 
    std::unordered_map<std::string, std::string> aliases;
    const std::string filename = ".aliases";
    const std::string postfix = " $ ";
    const std::string ANSI_COLOR_BLUE = "\033[34m";
    const std::string ANSI_COLOR_RESET = "\033[0m";

    // functions 
    std::string shorten_path(const std::string& path);
    std::vector<std::string> parse_line(const std::string& line);

public:
    ExecResult load_aliases();
    void replace_alias(std::vector<std::string>& tokens);
    std::string build_prompt(const std::string &path);
    char * get_input(const char * path);
};

