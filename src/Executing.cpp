#include "Executing.hpp"
#include <unordered_map>
#include <unistd.h>

namespace CommandExecuting
{
    static ExecResult builtin_cd(const Command& cmd);
    static ExecResult builtin_exit(const Command& cmd);

    static std::unordered_map<std::string, BuiltinFn> builtins =
    {
        {"cd", builtin_cd},
        {"exit", builtin_exit}
    };

    ExecResult execute_builtin(const Command& cmd)
    {
        if (cmd.arg.empty())
        {
            return ExecResult::ERROR;
        }
        auto it = builtins.find(cmd.arg[0]);
        return it->second(cmd);
    }

    bool is_builtin(const Command& cmd)
    {
        if (cmd.arg.empty())
            return false;

        return builtins.find(cmd.arg[0]) != builtins.end();
    }

    //Builtin implementations

    static ExecResult builtin_cd(const Command& cmd)
    {
        char buffer[1024];
        const char* dir_name;
        if (cmd.arg.size() != 2)
        {
            return ExecResult::ERROR;
        }
        dir_name = cmd.arg[1].c_str();

        if (chdir(dir_name) == 0)
        {
            if(getcwd(buffer,1024) != NULL)
            {
                std::cout << buffer << std::endl;
            }
            return ExecResult::OK;
        }
        else 
        {
           std::cerr << dir_name << " : Directory not found." << std::endl; 
        }
        
        return ExecResult::ERROR;
    }

    static ExecResult builtin_exit(const Command&)
    {
        return ExecResult::EXIT;
    }
}