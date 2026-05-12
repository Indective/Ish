#pragma once
#include "CommandModel.hpp"
#include <unordered_map>
#include <string>
#include <optional>
#include <unistd.h>

class ShellContext
{
private:
    static std::unordered_map<std::string, std::string> aliases;

    static constexpr const char* postfix = " $ ";
    static constexpr const char* blue = "\033[34m";
    static constexpr const char* reset = "\033[0m";
    static constexpr const char* filename = "/.ish_aliases";

    std::string shorten_path(const std::string& path);
    std::optional<std::vector<std::string>> parse_line(const std::string& line, const bool& is_alias_line);
    bool check_syntax(const std::vector<std::string> &tokens, const bool &in_quote, const bool &found_quote, const std::string& line);

public:
    static pid_t shell_pid;

    void install_sigchld();
    void install_sigint();
    void init_shell();
    void load_aliases();
    void replace_alias(Command& cmd);
    void reload_aliases(const std::string& path);

    std::string build_prompt(const std::string& path);
    const char * get_input(const char * path);
    const char * get_full_path();
};

