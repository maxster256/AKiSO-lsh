/*
 * default_functions.c
 * Configure the built-in shell functions
 */

#include "default_functions.h"


/*
  List of built-in shell functions, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

// Array of function pointers
int (*builtin_func[]) (char *[]) = {
  &change_directory,
  &show_help,
  &exit_shell
};

int number_of_builtin_functions() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
 * cd
 * Change the directory
 */
int change_directory(char* args[]) {

  if (args[1] == NULL) {
    // If no path is provided after the call to the function, go to home directory
  	chdir(getenv("HOME"));
  	return 1;
  }
  else {
    // Change to the directory provided by the user
  	if (chdir(args[1]) == -1) { // Handles the situation when desired directory is nonexistant
  		printf("%s: directory does not exist\n", args[1]);
      return -1;
  	}
  }
  return 0;
}

/*
 * help
 * Presents the user with the short help manual for lsh
 */
int show_help(char *args[]) {
  int i;
  printf("This is lsh - a bash implementation in C\n");
  printf("\nTo run the command:\n- Type the name of the command\n- Type the arguments needed to run the command\n- Hit the return key\n");
  printf("\nYou can also use the built-in commands from the list below:\n");

  for (i = 0; i < number_of_builtin_functions(); i++) {
    printf("- %s\n", builtin_str[i]);
  }

  printf("\nIn order to get more support about specific commands,\ntype man and the name of the command, eg. man rm\n");
  return 1;
}

/*
 * exit
 * Quits the shell
 */
int exit_shell(char *args[]) {
  exit(0);
}
