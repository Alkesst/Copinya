#include <stdio.h>
#include <stdlib.h> 
int main() {
    int status;
    int pid = fork();
    //printf("pid %d\n", pid);
    if (pid != 0) {
        // padre
        wait(&status);
    } else {
        // hijo
        while (1) {
            printf("A");
            fflush(NULL);
            sleep(1);
        }
    }
}
