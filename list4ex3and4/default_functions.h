/*
 * default_functions.h
 * Configure the built-in shell functions
 */

#include "lsh.h"

// Declares the built-in shell functions
int change_directory(char* args[]);
int show_help(char *args[]);
int exit_shell(char *args[]);

// Helper functions
int number_of_builtin_functions();
