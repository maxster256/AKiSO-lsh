// Lista 4
// Zadanie 2
#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>

void signal_handler(int signo) {
    printf("\nSignal %d was received\n", signo);
    if (signo == 2)
      exit(0);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < 32; i++)
      if (signal(i, signal_handler) == SIG_ERR)
          printf("Warning: Unable to catch signal %d\n", i);

    // Make the process sleep for a while so we'll be able to actually see something going on.
    while(1)
        sleep(1);

    return 0;
}
