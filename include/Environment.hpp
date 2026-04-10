#include "Command.hpp"
#include <unordered_map>
#include <string>

class Environment
{
private:
    // variables 
    static std::unordered_map<std::string, std::string> aliases;
    const std::string filename = ".aliases";
    const std::string postfix = " $ ";
    const std::string ANSI_COLOR_BLUE = "\033[34m";
    const std::string ANSI_COLOR_RESET = "\033[0m";

    // functions 
    std::string shorten_path(const std::string& path);
    std::vector<std::string> parse_line(const std::string& line);
    bool check_syntax(const std::vector<std::string>& tokens);

public:
    void load_aliases();
    void replace_alias(Command &cmd);
    std::string build_prompt(const std::string &path);
    char * get_input(const char * path);
};

