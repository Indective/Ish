#include "ShellContext.hpp"
#include "Parser.hpp"
#include "JobControl.hpp"
#include "Signal.hpp"
#include "AliasManager.hpp"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <optional>
#include <filesystem>

pid_t ShellContext::shell_pid = 0;

void ShellContext::init_shell()
{
    Signal::install_sigint(); 
    Signal::install_sigchld(); 
    Signal::install_sigstp(); 
    Signal::install_sigsttin(); 
    Signal::install_sigttou();

    shell_pid = getpid();
    setpgid(0, 0);

    tcsetpgrp(STDIN_FILENO, shell_pid);
    AliasManager::load_aliases();

    rl_catch_signals = 0; // stop readline form catching signals
}

