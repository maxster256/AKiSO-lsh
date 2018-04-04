/*
 * lsh
 * A shell implementation written in C language
 * by PN, © 2017 nows
 */

#include "lsh.h"

/*
 * Initialize the shell.
 */
void initialize_shell() {

    // Get the PID of the shell process
    SHELL_PID = getpid();

    // Check if the shell is running in the interactive mode
    // The shell is interactive if STDIN is the terminal
    SHELL_IS_INTERACTIVE = isatty(STDIN_FILENO);

    if (SHELL_IS_INTERACTIVE) {
      // Send the SIGTTIN signal while the process is in the background.
      // A process cannot read from the user’s terminal while it is running as a background job.
      while (tcgetpgrp(STDIN_FILENO) != (SHELL_PGID = getpgrp())) // When any process in a background job tries to read from the terminal, all of the processes in the job are sent a SIGTTIN signal.
					kill(SHELL_PID, SIGTTIN); // The default action for this signal is to stop the process.

	    // Set the signal handlers for SIGCHILD and SIGINT
			act_child.sa_handler = sigchild_signal_handler;
			act_int.sa_handler = sigint_signal_handler;

			/* The sigaction structure is as follows:
			struct sigaction {
				void (*sa_handler)(int);
				void (*sa_sigaction)(int, siginfo_t *, void *);
				sigset_t sa_mask;
				int sa_flags;
				void (*sa_restorer)(void);

			}*/

			sigaction(SIGCHLD, &act_child, 0);
			sigaction(SIGINT, &act_int, 0);

			// Configure shell's own process group
			setpgid(SHELL_PID, SHELL_PID); // Set the shell's process as the process group leader
			SHELL_PGID = getpgrp();

      if (SHELL_PID != SHELL_PGID) {
					fprintf(stderr, "lsh error: the shell's process is not the process group's leader");
					exit(EXIT_FAILURE);
			}

			// Make the process group the foreground process group on the terminal
      // In other words, start controlling the terminal
			tcsetpgrp(STDIN_FILENO, SHELL_PGID);

			// Get the default terminal attributes
			tcgetattr(STDIN_FILENO, &SHELL_TERMINAL_MODES);

			// Get the current directory that will be used in different methods
			current_directory = (char*) calloc(1024, sizeof(char));
    }
    else {
      fprintf(stderr, "lsh error: unable to make the shell run in interactive mode\n");
      exit(EXIT_FAILURE);
    }
}

/**
 * Handle the display of the prompt for the user:
 * Prompt's format: [username]@[hostname] [current directory]
 */
void display_shell_prompt() {
	char hostn[MAX_CHARS_PER_LINE] = "";

  gethostname(hostn, sizeof(hostn));
	printf("%s@%s %s > ", getenv("LOGNAME"), hostn, getcwd(current_directory, MAX_CHARS_PER_LINE));
}

/**
* Launches a command.
* Command can be either run in the foreground, or the background
*/
void run_command(char **args, int background){
  int err = -1;

  if ((pid = fork()) == -1){
   fprintf(stderr, "lsh error: child process could not be created\n");
   return;
  }

  // If the PID is equal to 0, it means that we're in a child process
  if (pid == 0) {

    // Make child process ignore SIGINT signals
    // They will be handled by the parent process with sigint_signal_handler() method
    signal(SIGINT, SIG_IGN);

    // Set the child's parent enviroment value to
    // parent=<pathname>/lsh
    setenv("parent", getcwd(current_directory, 1024), 1);

    // If the user tries to launch commands/programs which are not available, return an error
    if (execvp(args[0] ,args) == err) {
      printf("lsh: command not found");
      kill(getpid(), SIGTERM);
    }
  }
  if (background == 0) {
    // The following code will be executed by the parent process
    // If the process is not requested to be in background, we wait for the child to finish.
    waitpid(pid, NULL, 0);
  }
  else {
    // In order to create a background process, the current process
    // should just skip the call to wait. The SIGCHILD handler
    // sigchild_signal_handler will take care of the returning values
    // of the childs.
    printf("lsh: process created with PID: %d\n", pid);
	 }
}

/**
* Manages various input/output redirections to/from files
*/
void file_input_output_handler(char * args[], char* inputFile, char* outputFile, int option) {

	int err = -1;

	if ((pid = fork()) == -1){
		fprintf(stderr, "lsh error: child process could not be created\n");
		return;
	}
	if (pid==0) {
		if (option == 0) {
      // This option (0) andles the output redirection only

      // Open/create the file truncating it at 0, for write only
    	int output_file_descriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);

      // Redirect the standard output to the file
    	dup2(output_file_descriptor, STDOUT_FILENO);
    	close(output_file_descriptor);
		}
    else if (option == 1) {
      // This option (1) manages both input and output redirections

      // Open the read-only file (which is STDIN, actually)
			int input_file_descriptor = open(inputFile, O_RDONLY, 0600);

      // Redirect the standard input to the appropriate file
			dup2(input_file_descriptor, STDIN_FILENO);
			close(input_file_descriptor);

      // Open the file to which the output will be passed
			int output_file_descriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);

      // Redirect the standard output to the file
      dup2(output_file_descriptor, STDOUT_FILENO);
			close(output_file_descriptor);
		}
    else if (option == 2)  {
      // This option manages the STDERR redirections

      int output_file_descriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);

      // Redirect the standard output to the file
    	dup2(output_file_descriptor, 2);
    	close(output_file_descriptor);
    }
    else {
      fprintf(stderr, "lsh: error while handling input/output; invalid arguments were provided");
    }

    // Set the child's parent enviroment value to
    // parent=<pathname>/lsh
		setenv("parent", getcwd(current_directory, 1024), 1);

		if (execvp(args[0], args) == err){
			printf("lsh: error while handling input/output");
			kill(getpid(), SIGTERM);
		}
	}
  // If the process is not requested to be in background, we wait for the child to finish.
  waitpid(pid, NULL, 0);
}

/**
* Manages operations involving pipes.
*/
void pipe_handler(char * args[]) {
  // Declares the file descriptors
  // The first element in the array is the output of the pipe, the second - input
  int first_file_descriptor[2];
  int second_file_descriptor[2];

  int piped_commands_count = 0, err = -1;
  bool has_reached_end = false;
  char *command[MAX_ARGS_PER_LINE];
  pid_t pid;

  // Variables used for the different loops
  int i = 0;
  int token_count = 0, commands_counter_iterator = 0, pipe_configurer_iterator = 0;

  // Calculates the number of different commands to execute
  // The commands are separated by '|' (pipe sign)
  while (args[commands_counter_iterator] != NULL){
  	if (strcmp(args[commands_counter_iterator],"|") == 0)
  		piped_commands_count++;

    commands_counter_iterator++;
  }
  piped_commands_count++;

  // Main loop of this method.

  // For each command between '|', the
  // pipes will be configured and standard input and/or output will
  // be replaced. Then the entire command will be executed.
  while (args[pipe_configurer_iterator] != NULL && !has_reached_end) {
  	token_count = 0;
  	// We use an auxiliary array of pointers to store the command
  	// that will be executed on each iteration
  	while (strcmp(args[pipe_configurer_iterator], "|") != 0) {
  		command[token_count] = args[pipe_configurer_iterator];
  		pipe_configurer_iterator++;

  		if (args[pipe_configurer_iterator] == NULL) {
  			// 'has_reached_end' variable used to keep the program from entering
  			// again in the loop when no more arguments are found
  			has_reached_end = true;
  			token_count++;
  			break;
  		}
  		token_count++;
  	}
    // Set the last token to NULL to indicate its' end.
    // It will be later recognized by the exec() function.
  	command[token_count] = NULL;
  	pipe_configurer_iterator++;

    // Sets different file descriptors for pipes inputs and outputs,
    // depending on the current iteration.
    // Because of that, a pipe will be shared between two iterations,
    // allowing to connect inputs and outputs of two different commands.
  	if (i % 2 != 0)
    	pipe(first_file_descriptor); // for odd i
    else
      pipe(second_file_descriptor); // for even i

  	pid = fork();

  	if (pid == -1) {
  		if (i != piped_commands_count - 1) {
  			if (i % 2 != 0) {
  				close(first_file_descriptor[1]); // for odd i
  			}
        else {
  				close(second_file_descriptor[1]); // for even i
  			}
  		}
  		fprintf(stderr, "lsh: child process could not be created\n");
  		return;
  	}
  	if (pid == 0) {
  		if (i == 0) {
        // If it's the first command we're working on
  			dup2(second_file_descriptor[1], STDOUT_FILENO);
  		}
  		else if (i == piped_commands_count - 1) {
        // If it's the last command, depending on whether it
    		// is placed in an odd or even position, the standard input
    		// for one pipe or another will be replaced. The standard
    		// output will not be changed, because we want to show
    		// final output in the terminal.
        if (piped_commands_count % 2 != 0) {
  				dup2(first_file_descriptor[0], STDIN_FILENO); // for odd number of commands
  			}
        else {
  				dup2(second_file_descriptor[0], STDIN_FILENO); // for even number of commands
  			}
  		}
      else {
        // If we are in a command that is in the middle, we will
    		// have to use two pipes, one for input and another for
    		// output. The position is also important in order to choose
    		// which file descriptor corresponds to each input/output
  			if (i % 2 != 0) {
  				dup2(second_file_descriptor[0], STDIN_FILENO); // for odd i
  				dup2(first_file_descriptor[1], STDOUT_FILENO);
  			}
        else {
  				dup2(first_file_descriptor[0], STDIN_FILENO); // for even i
  				dup2(second_file_descriptor[1], STDOUT_FILENO);
  			}
  		}

      if (execvp(command[0], command) == err){
  			kill(getpid(), SIGTERM);
  		}
  	}

  	// Closes the descriptors on parent
  	if (i == 0) {
  		close(second_file_descriptor[1]);
  	}
  	else if (i == piped_commands_count - 1) {
  		if (piped_commands_count % 2 != 0){
  			close(first_file_descriptor[0]);
  		}
      else {
  			close(second_file_descriptor[0]);
  		}
  	}
    else{
  		if (i % 2 != 0){
  			close(second_file_descriptor[0]);
  			close(first_file_descriptor[1]);
  		}
      else{
  			close(first_file_descriptor[0]);
  			close(second_file_descriptor[1]);
  		}
  	}

  	waitpid(pid, NULL, 0);
  	i++;
  }
}

/**
 * Handles the command entered via standard input
 */
int parse_command(char * args[]) {
 	int i = 0, j = 0, auxillary_args_count;
  bool is_background = false;

 	char *auxillary_args[256];

 	// We look for the special characters and separate the command itself
 	// in a new array for the arguments
 	while ( args[j] != NULL ){
 		if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
 			break;
 		}
 		auxillary_args[j] = args[j];
 		j++;
 	}

  // Check if the user wants to run a built-in command instead of a Unix program
  for (int k = 0; k < number_of_builtin_functions(); k++) {
    if (strcmp(auxillary_args[0], builtin_str[k]) == 0) {
      //printf("Detected builtin...");
      return (*builtin_func[k])(args);
    }
  }

	while (args[i] != NULL && is_background == 0) {
		if (strcmp(args[i], "&") == 0) {
      // If background execution is needed (last argument '&') the loop will be exited.
			is_background = true;
		}
    else if (strcmp(args[i], "|") == 0) {
      // If the '|' is detected, the pipe handler is called to handle the execution of the command.
			pipe_handler(args);
			return 1;
		}
    else if(strcmp(args[i], "2>") == 0) {
      // If the '2>' (Error redirection) is detected, its' validity is checked,
      // and an appropriate method is called.

      if (args[i+1] == NULL) {
        fprintf(stderr, "lsh: not enough input arguments for error redirection\n");
				return -1;
			}
			file_input_output_handler(auxillary_args, NULL, args[i+1], 2);
			return 1;
		}
    else if (strcmp(args[i], "<") == 0) {
      // If the '<' (I/O redirection) is detected, its' validity is checked,
      // and an appropriate method is called.
			auxillary_args_count = i+1;

			if (args[auxillary_args_count] == NULL || args[auxillary_args_count+1] == NULL || args[auxillary_args_count+2] == NULL ){
				fprintf(stderr, "lsh: not enough input arguments for I/O redirection\n");
				return -1;
			}
      else{
				if (strcmp(args[auxillary_args_count+1], ">") != 0) {
					printf("lsh usage: expected '>' and found %s\n", args[auxillary_args_count+1]);
					return -2;
				}
			}
			file_input_output_handler(auxillary_args, args[i+1], args[i+3], 1);
			return 1;
		}
    // If the '>' (I/O redirection) is detected, its' validity is checked,
    // and an appropriate method is called.
		else if (strcmp(args[i],">") == 0) {
			if (args[i+1] == NULL){
        fprintf(stderr, "lsh: not enough input arguments for I/O redirection\n");
				return -1;
			}
			file_input_output_handler(auxillary_args, NULL, args[i+1], 0);
			return 1;
		}
		i++;
	}

  // Adds NULL at the end of the command, indicating to the parser that it's finished.
	auxillary_args[i] = NULL;

  // Runs the command
	run_command(auxillary_args, is_background);

return 1;
}


/**
* Main method of our shell
*/
int main(int argc, char *argv[], char ** envp) {
  char line[MAX_CHARS_PER_LINE]; // Buffer for the data provided by the user, loaded from the standard input
	char *tokens[MAX_ARGS_PER_LINE]; // Array holding the different tokens
	int number_of_tokens;

  // Prepares prompt for the initalization
	SHOULD_NOT_REPRINT_PROMPT = false; // The prompt should be shown to the user
	pid = -10; // Assign an impossible value, so that if any problems with creating a process should occur, program will crash

	// Calls the initalize_shell() method and prepares the shell for the user
	initialize_shell();
	printf("\nWelcome to lsh.\nVersion 0.1\nCopyright © 1997-2017\n\n");

  // Sets the enviroment variable shell=<pathname>/lsh for the child process
	setenv("shell", getcwd(current_directory, 1024), 1);

	// Prepares the command loop
	while (true) {
    // Print the shell prompt if necessary
		if (!SHOULD_NOT_REPRINT_PROMPT)
      display_shell_prompt();

    SHOULD_NOT_REPRINT_PROMPT = false;

		// Empties the line buffer
		memset(line, '\0', MAX_CHARS_PER_LINE );

		// Waits for the user input
		fgets(line, MAX_CHARS_PER_LINE, stdin);

		// If no input is provided by the user, the loop continues
		if((tokens[0] = strtok(line," \n\t")) == NULL)
      continue;

		// If not, the tokens are read from the input and passed to the
		// parse_command() method as its' argument
		number_of_tokens = 1;

		while((tokens[number_of_tokens] = strtok(NULL, " \n\t")) != NULL)
      number_of_tokens++;

		parse_command(tokens);
	}

	exit(0);
}
