# Small Shell

## Program Functionality

Small Shell or `smallsh` is a minimal shell program that replicates some of the basic functionalities of a Unix shell. It supports the following features:

- **Command Execution**: Execute commands with arguments, including built-in commands and external programs. The shell supports command lines with a maximum length of 2048 characters and an maximum of 512 arguments.
- **Built-in Commands**:
  - `cd [directory]`: Change the current directory to the specified directory. If no directory is specified, it changes to directory specified in the HOME environment variable
  - `status`: Display the exit status of the last foreground process.
  - `exit`: Kills all processes or jobs started by the shell before the shell terminates itself.
- **Comments**:
  - Blank lines and lines starting with `#` are treated as comments and are ignored by the shell.
  - Mid-line comments, such as the C-style `//`, are not supported.
- **Foreground and Background Execution**:
  - Commands can be run in the foreground or background. Background commands are indicated by an `&` at the end of the command line.
- **Input and Output Redirection**:
  - Redirect input using `<` and output using `>`.
  - By default, `stdin` and `stdout` for background commands are set to `/dev/null`.
- **Signal Handling**:
  - `SIGINT (CTRL-C)`: The shell ignores `SIGINT`, but any foreground child process will terminate upon receiving this signal. If a foreground child process is terminated by `SIGINT`, the shell will print the signal number that caused the termination.
  - `SIGTSTP (CTRL-Z)`: The shell toggles between allowing and disallowing background processes. When `SIGTSTP` is received, the shell will display a message and enter a state where background processes are not allowed. Sending `SIGTSTP` again will revert this state, allowing background processes once more.

## Compilation Instructions

To compile the `smallsh` program, follow these steps:

1. Open a terminal and navigate to the directory containing the source code.
2. Run the following command to compile the program:

   ```sh
   gcc -o smallsh *.c
   ```

3. After compilation, you can run the shell with:

   ```sh
   ./smallsh
   ```
