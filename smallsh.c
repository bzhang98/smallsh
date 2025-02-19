#include "command_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_BG_PROCESSES 100

// Global variables to store background process IDs
pid_t bg_pids[MAX_BG_PROCESSES];
int bg_pids_count = 0;

// Global variables to indicate whether the parent process is blocked and whether to ignore the "&" operator
volatile sig_atomic_t blocked = false;
volatile sig_atomic_t ignore_bg = false;

void cd(char *path)
{
  if (!path)
  {
    chdir(getenv("HOME"));
    return;
  }

  if (chdir(path) == -1)
  {
    perror("cd");
  }
}

void status(int status)
{
  if (WIFEXITED(status))
  {
    printf("exit value %d\n", WEXITSTATUS(status));
    fflush(stdout);
  }
  else
  {
    printf("terminated by signal %d\n", WTERMSIG(status));
    fflush(stdout);
  }
}

// Code adapted from Module 7 - Processes II Exploration: Processes and I/O
void redirect_io(struct command_line *command)
{
  if (command->input_file)
  {
    int sourceFD = open(command->input_file, O_RDONLY);
    if (sourceFD == -1)
    {
      perror("open");
      exit(EXIT_FAILURE);
    }
    if (dup2(sourceFD, STDIN_FILENO) == -1)
    {
      perror("dup2");
      exit(EXIT_FAILURE);
    }
  }

  if (command->output_file)
  {
    int targetFD = open(command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (targetFD == -1)
    {
      perror("open");
      exit(EXIT_FAILURE);
    }
    if (dup2(targetFD, STDOUT_FILENO) == -1)
    {
      perror("dup2");
      exit(EXIT_FAILURE);
    }
  }
}

void add_bg_pid(pid_t pid)
{
  if (bg_pids_count < MAX_BG_PROCESSES)
  {
    bg_pids[bg_pids_count++] = pid;
  }
}

void remove_bg_pid(int index)
{
  for (int i = index; i < bg_pids_count - 1; i++)
  {
    bg_pids[i] = bg_pids[i + 1];
  }
  bg_pids_count--;
}

void reap_background_processes()
{
  int status;

  for (int i = 0; i < bg_pids_count;)
  {
    pid_t pid = waitpid(bg_pids[i], &status, WNOHANG);
    if (pid > 0)
    {
      if (WIFEXITED(status))
      {
        printf("background pid %d is done: exit value %d\n", pid, WEXITSTATUS(status));
        fflush(stdout);
      }
      else
      {
        printf("background pid %d is done: terminated by signal %d\n", pid, WTERMSIG(status));
        fflush(stdout);
      }
      remove_bg_pid(i);
    }
    else
    {
      i++;
    }
  }
}

void handle_sigtstp(int signo)
{
  if (!ignore_bg)
  {
    char message[] = "\nEntering foreground-only mode (& is now ignored)\n";
    write(STDOUT_FILENO, message, sizeof(message) - 1);
  }
  else
  {
    char message[] = "\nExiting foreground-only mode\n";
    write(STDOUT_FILENO, message, sizeof(message) - 1);
  }

  // Print the prompt only if the parent process is not blocked and is waiting for user input
  if (!blocked)
  {
    write(STDOUT_FILENO, ": ", 2);
  }

  ignore_bg = !ignore_bg;
}

int main()
{
  struct command_line *curr_command;
  int childStatus = 0;

  // Ignore SIGINT in parent process
  struct sigaction SIGINT_action = {0};
  SIGINT_action.sa_handler = SIG_IGN;
  sigaction(SIGINT, &SIGINT_action, NULL);

  // Register SIGTSTP signal handler
  struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = handle_sigtstp;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  while (true)
  {
    // Reap background processes before returning to the prompt
    reap_background_processes();

    curr_command = parse_input();

    // Skip empty lines and comments
    if (!curr_command)
    {
      continue;
    }

    // Handle built-in command "exit"
    if (!strcmp(curr_command->argv[0], "exit"))
    {
      break;
    }

    // Handle built-in command "cd"
    if (!strcmp(curr_command->argv[0], "cd"))
    {
      cd(curr_command->argc == 1 ? NULL : curr_command->argv[1]);
      continue;
    }

    // Handle built-in command "status"
    if (!strcmp(curr_command->argv[0], "status"))
    {
      status(childStatus);
      continue;
    }

    // Handle other commands
    pid_t pid = fork();
    switch (pid)
    {
    case -1:
      perror("fork()\n");
      return EXIT_FAILURE;
      break;
    case 0:                                  // Child process
      if (curr_command->is_bg && !ignore_bg) // Run in background
      {
        // Redirect standard output to /dev/null if not specified
        if (curr_command->output_file == NULL)
        {
          curr_command->output_file = "/dev/null";
        }
        // Redirect standard input to /dev/null if not specified
        if (curr_command->input_file == NULL)
        {
          curr_command->input_file = "/dev/null";
        }
      }
      else // Run in foreground
      {
        SIGINT_action.sa_handler = SIG_DFL;
        sigaction(SIGINT, &SIGINT_action, NULL);
      }

      // ignore SIGTSTP in child process
      SIGTSTP_action.sa_handler = SIG_IGN;
      sigaction(SIGTSTP, &SIGTSTP_action, NULL);

      redirect_io(curr_command);
      execvp(curr_command->argv[0], curr_command->argv);
      perror("execvp");
      return EXIT_FAILURE;
      break;
    default:                                 // Parent process
      if (!curr_command->is_bg || ignore_bg) // Wait for child process
      {
        // Block SIGTSTP in parent process while waiting for child process
        // Adapted from https://stackoverflow.com/questions/25261/set-and-oldset-in-sigprocmask
        sigset_t mask, prev_mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGTSTP);
        sigprocmask(SIG_BLOCK, &mask, &prev_mask);

        // Set global blocked flag to indicate that the parent process is blocked
        blocked = true;

        waitpid(pid, &childStatus, 0);

        // Print the exit status of the child process if it was terminated by a signal
        if (WIFSIGNALED(childStatus))
        {
          printf("terminated by signal %d\n", WTERMSIG(childStatus));
          fflush(stdout);
        }

        // Unblock SIGTSTP in parent process
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        blocked = false;
      }
      else // Allow child process to run in background and print its PID
      {
        add_bg_pid(pid);
        printf("background pid is %d\n", pid);
        fflush(stdout);
      }
      break;
    }
  }
  free_command(curr_command);
  return EXIT_SUCCESS;
}