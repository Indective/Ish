#pragma once
#include "Types.hpp"
#include <unordered_map>
#include <string>
#include <optional>
#include <unistd.h>

class ShellContext
{
public:
    static pid_t shell_pid;
    
    void init_shell();
};

