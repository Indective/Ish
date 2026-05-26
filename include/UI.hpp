#pragma once

#include <iostream>

namespace UI
{
    std::string shorten_path(const std::string& path);
    void CleanUI();
    std::string build_prompt(const std::string& path);
    const char * get_input(const char * path);
    const char * get_full_path();
}