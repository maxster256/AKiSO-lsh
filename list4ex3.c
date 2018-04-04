// List 4
// Exercise 3
// lsh
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TOKENS_BUFFER_SIZE 64
#define TOKENS_SEPARATORS " \t\r\n\a"

/*
  Declares the built-in shell commands.
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

// TO CHECK OUT LATER - CONFUSING!
// Array of function pointers
int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Implementations of built-in functions.
*/
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  }
  else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("This is lsh - a bash implementation in C\n");
  printf("\nTo run the command:\n- Type the name of the command\n- Type the arguments needed to run the command\n- Hit the return key\n");
  printf("\nYou can also use the built-in commands from the list below:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("- %s\n", builtin_str[i]);
  }

  printf("\nIn order to get more support about specific commands,\ntype man and the name of the command, eg. man rm\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0;
}

/*
  Reads the line provided from the standard input.
*/
char *read_line(void) {
  char *input_line = NULL;
  ssize_t buffer_size = 0; // have getline allocate a buffer for us

  getline(&input_line, &buffer_size, stdin);
  return input_line;
}
/*
  Converts the line into tokens.
*/
char **tokenize_line(char *line) {
  int bufsize = TOKENS_BUFFER_SIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOKENS_SEPARATORS);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += TOKENS_BUFFER_SIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOKENS_SEPARATORS);
  }
  tokens[position] = NULL;
  return tokens;
}

/*
  Runs the given program.
*/
int launch_with(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Instructions for the child process, in which the command given by the user will run
    // execvp expects a program name and an array (v), will figure out the program path on its' own

    // Perform the execvp and handle the execution error
    if (execvp(args[0], args) == -1)
      perror("lsh");

    exit(EXIT_FAILURE);
  }
  else if (pid < 0) {
    // Handle the forking error by printing it - there's nothing more to do here
    perror("lsh");
  }
  else {
    // Instructions for the parent process
    // (we're here, which means, that fork() has been executed successfully)
    do {
      // Wait for the child process to be either exited or killed
      wpid = waitpid(pid, &status, WUNTRACED);
    }
    // WIFEXITED queries the status of the child process to see, if it's ended normally
    // WIFSIGNALED queries the status of the child process to see, if it's ended abnormally
    while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  // Check if the user wants to run a built-in command instead of a Unix program
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return launch_with(args);
}

/*
  Handles the provided command.
  1. Reading: Read the command from standard input.
  2. Parsing: Separate the command string into a program and arguments.
  3. Execution: Run the parsed command.
*/
void command_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = read_line();
    args = tokenize_line(line);
    status = execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  command_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
