## Shell structure :

**Prompt** : A symbol or text ($, >) indicating the shell is ready for input.

**Command Line** : The input area where users type commands, arguments, and options.

**Command Interpreter (Parser)** : Breaks down input into tokens (commands, arguments) and determines how to execute them.

**Execution Environment** : Manages environment variables, shell built-ins (like cd), and path searching for executables.

**I/O Redirection and Piping** : Facilitates redirecting input/output (<, >) and chaining commands (|).

**Scripting Engine** : Enables automation by interpreting files containing sequences of commands.

## Operation Flow : 

**Read** : The shell displays a prompt and reads user input.

**Evaluate** : It tokenizes the input, checks for aliases or built-ins, and expands variables.

**Execute** : The shell forks a process, executes the command, and waits for it to finish.

**Print** : It displays output or errors to the terminal



to be done : 
aliases       done, working
background processes      
automations scripts
job control
pipes
replace /home/[username] with ~ when displaying current working directory    done, working
press arrow keys to get last used commands. (could be done fastly)       done, working
reload aliases when changing .aliases
fix issue with exit command, as right now it is needed to run this command the same number times the command execution for other commands failed throught the shell session.

 