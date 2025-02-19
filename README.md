# Small Shell

## Program Functionality

`smallsh` is a minimal shell program that replicates some of the basic functionalities of a Unix shell. It supports the following features:

- **Command Execution**: Execute commands with arguments, including built-in commands and external programs.
- **Built-in Commands**:
  - `cd [directory]`: Change the current directory to the specified directory. If no directory is specified, it changes to the home directory.
  - `status`: Display the exit status of the last foreground process.
  - `exit`: Exit the shell.
- **Foreground and Background Execution**:
  - Commands can be run in the foreground or background. Background commands are indicated by an `&` at the end of the command line.
- **Input and Output Redirection**:
  - Redirect input using `<` and output using `>`.
- **Signal Handling**:
  - Handle `SIGINT` and `SIGTSTP` signals to control foreground and background process execution.

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
