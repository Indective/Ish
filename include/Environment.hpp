#include "Command.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

class Environment
{
private:
    std::unordered_map<std::string, std::string> aliases;
    std::string filename = ".aliases";
    std::vector<std::string> parse_line(const std::string& line);
    std::string handle_qoutes(const std::string& qouted_string);
    bool check_syntax(const std::vector<std::string> args);
public:
    ExecResult load_aliases();
    bool has_alias(const Command& cmd);
};

