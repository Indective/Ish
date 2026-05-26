#pragma once
#include <iostream>
#include <signal.h>

namespace Signal
{
    extern volatile sig_atomic_t sigchld;
    extern volatile sig_atomic_t sigint;

    void sigchldHandler(int);
    void sigintHandler(int);
    
    void install_sigchld();
    void install_sigint();
    void install_sigttou();
    void install_sigstp();
    void install_sigsttin();
}