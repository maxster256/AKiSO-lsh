/*
 * signalhandlers.h
 * Configure the actions invoked upon the signal being sent to the shell.
 */

#include "lsh.h"

// Declares signal handlers
void sigchild_signal_handler(int p); // SIGCHILD signal handler
void sigint_signal_handler(int p); // SIGINT signal handler
