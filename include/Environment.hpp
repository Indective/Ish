#include "Command.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

class Environment
{
private:
    std::unordered_map<std::string, std::string> aliases;
    const std::string filename = ".aliases";
    const std::string prefix = " $ ";
    std::vector<std::string> parse_line(const std::string& line);
public:
    ExecResult load_aliases();
    void replace_alias(std::vector<std::string>& tokens);
    std::string shorten_path(const std::string& path);
};

