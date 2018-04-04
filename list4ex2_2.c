// Lista 4
// Zadanie 2.2
// Czy jest możliwe wysłać sygnał SIGKILL, lub inny do procesu init (PID 1) czyli np. kill -9 1 (nawet będąc rootem)?

// init ignoruje SIGINT, SIGKILL
// Gdyby nie ignorował, kernel wymusiłby "wysypanie się" systemu

#include <sys/types.h>
#include <signal.h>

int main(int argc, char *argv[]) {

    kill(1, SIGKILL);
    kill(1, SIGINT);

    return 0;
}
