// List 4
// Excercise 1

// (5pt) Napisz program w języku C, który uruchomi powłokę (Bash) z prawami roota.
// Po kompilacji programu można ustawić (z poziomu roota) dowolne atrybuty (np. patrz SUID).
// Następnie już z poziomu dowolnego użytkownika uruchamiając program uruchamia się konsola administratora,
// podobnie jak sudo /bin/bash (bez wprowadzania hasła).

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    setuid(0);   // you can set it at run time also
    system( "/bin/bash" );
    return 0;
}
