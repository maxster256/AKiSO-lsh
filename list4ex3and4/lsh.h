/*
 * lsh.h
 * Header file for the shell implementation
 */

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>

// Internal depedencies
#import "signal_handlers.c"
#import "default_functions.c"

// Variables
// Definitions
#define MAX_ARGS_PER_LINE 256 // Maximum number of tokens to enter in one command
#define MAX_CHARS_PER_LINE 1024 // Maximum amount of characters to enter by the user

// Shell's PID, PGID and terminal modes
static pid_t SHELL_PID;
static pid_t SHELL_PGID;
static bool SHELL_IS_INTERACTIVE;
static struct termios SHELL_TERMINAL_MODES;

// Shell's signal handlers
struct sigaction act_child;
struct sigaction act_int;

// Info about current instance of shell
static char* current_directory;
bool SHOULD_NOT_REPRINT_PROMPT;

// Current PID
pid_t pid;

// Method declarations

void initialize_shell();
void display_shell_prompt();
void run_command(char **args, int background);
void file_input_output_handler(char * args[], char* inputFile, char* outputFile, int option);
void pipe_handler(char * args[]);

// Function declarations
int parse_command(char * args[]);
