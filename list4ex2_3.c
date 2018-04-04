// Lista 4
// Zadanie 2.3
// Czy sygnały są kolejkowane?
// Np. napisz program testowy wysyłający wiele razy do danego procesu sygnał (np. SIGUSR1) i zobacz czy wszystkie dotarły.

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int counter = 0;

void sigusr1_signal_handler(int signal_number) {
  printf("Received %d\n", counter);
  counter++;
}

int main() {
  pid_t pid;
  pid = fork();

  if (pid == -1) {
    perror("An error has occured when forking the process.\n");
    return 0;
  }
  else if (pid == 0) {
    for (int i = 0; i < 1000; i++) {
      printf("Sending %d\n", i);
      kill(getppid(), SIGUSR1);
    }
    return 0;
  }
  else {
    signal(SIGUSR1, sigusr1_signal_handler);
    while(1);
  }
  return 0;
}
