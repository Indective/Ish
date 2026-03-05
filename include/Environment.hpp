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
public:
    ExecResult load_aliases();
    std::vector<std::string> replace_alias(std::vector<std::string>& tokens);
};

