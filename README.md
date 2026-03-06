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
aliases
background processes
automations scripts
maybe windows compatibility
replace /home/[username] with ~ when displaying current working directory
 

aliases : 
let user save aliases at ish/.aliases
load aliases into memory on startup
whenever a user enters a command, check for aliases
if found, replace the alias with the corresponding command before executing it
move command to the execution section.

alias structure : 

alias {alias name} = "{command}"
0         1         2      3

use camke .. and make run to compile and run
dw i will change this README when i am atleast semi done with the project