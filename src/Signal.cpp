#include "Signal.hpp"

#include <iostream>
#include <unistd.h>

namespace Signal
{
    volatile sig_atomic_t sigchld = 0;
    volatile sig_atomic_t sigint = 0;

    void sigchldHandler(int)
    {
        sigchld = 1;
    }

    void sigintHandler(int)
    {
        sigint = 1;
    }
    
    void install_sigchld()
    {
        struct sigaction sa{};
        sa.sa_handler = sigchldHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

        sigaction(SIGCHLD, &sa, nullptr);
    }

    void install_sigint()
    {
        struct sigaction sa{};
        sa.sa_handler = sigintHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGINT, &sa, nullptr);
    }

    void install_sigttou()
    {
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGTTOU, &sa, nullptr);
    }

    void install_sigstp()
    {
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGTSTP, &sa, nullptr);
    }

    void install_sigsttin()
    {
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGTTIN, &sa, nullptr);
    }

}