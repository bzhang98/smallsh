#include "command_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Code adapted from sample_parser.c provided in the assignment description
struct command_line *parse_input()
{
  char input[INPUT_LENGTH];

  struct command_line *curr_command = (struct command_line *)calloc(1, sizeof(struct command_line));

  printf(": ");
  fflush(stdout);
  fgets(input, INPUT_LENGTH, stdin);

  char *token = strtok(input, " \n");

  // Skip empty lines and comments
  if (!token || token[0] == '#')
  {
    return NULL;
  }

  while (token)
  {
    if (!strcmp(token, "<"))
    {
      curr_command->input_file = strdup(strtok(NULL, " \n"));
    }
    else if (!strcmp(token, ">"))
    {
      curr_command->output_file = strdup(strtok(NULL, " \n"));
    }
    else if (!strcmp(token, "&"))
    {
      curr_command->is_bg = true;
    }
    else
    {
      curr_command->argv[curr_command->argc++] = strdup(token);
    }

    token = strtok(NULL, " \n");
  }

  return curr_command;
}

void free_command(struct command_line *command)
{
  for (int i = 0; i < command->argc; i++)
  {
    free(command->argv[i]);
  }

  free(command->input_file);
  free(command->output_file);
  free(command);
}