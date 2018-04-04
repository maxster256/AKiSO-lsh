/*
 * signalhandlers.c
 * Configure the actions invoked upon the signal being sent to the shell.
 */

#include "signal_handlers.h"

/*
 * SIGCHILD signal handler
 */
void sigchild_signal_handler(int p) {

    // Go into a loop to make sure that there are no more children which need to be handled.
    // WNOHANG (non-blocking call) makes sure that there are no problems if the child is cleaned up elsewhere.
    while (waitpid(-1, NULL, WNOHANG) > 0) {
      // printf("child %d terminated\n", pid);
    }
    printf("\n");
}

/*
 * SIGINT signal handler
 */
void sigint_signal_handler(int p) {

  // Send the SIGTERM signal to the child process
  if (kill(pid, SIGTERM) == 0) {
    printf("\nlsh: process %d received a SIGINT signal\n", pid);
    SHOULD_NOT_REPRINT_PROMPT = true;
  }
  else
    printf("\n");
}
