#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#define NUM_PROC 6
/* manejador_SIGTERM: saca un mensaje por pantalla y termina el proceso. */
void manejador_SIGUSR2(int sig) {
    printf("Terminación del proceso %d a petición del usuario \n", getpid());
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

/* manejador_SIGUSR1: presenta un número aleatorio por pantalla. */
void manejador_SIGUSR1(int sig) {
    printf("%d\n", rand());
    fflush(stdout);
}

int main(void) {
    struct sigaction act;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    pid_t pid;
    int i;


    /* Se arma la señal SIGTERM. */
    act.sa_handler = manejador_SIGUSR2;
    if (sigaction(SIGUSR2, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Se arma la señal SIGUSR1. */
    act.sa_handler = manejador_SIGUSR1;
    if (sigaction(SIGUSR1, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Se muestra el PID para facilitar el uso de kill.
    También se podría usar killall. */

    
    pid=fork();


    if(pid<1){
        perror("Error abriendo el fork\n");
        exit(EXIT_FAILURE);
    }

    if(pid==0){

        for(i=0; i<=NUM_PROC;i++){
            pid = fork();
            if (pid < 0) {
                perror("Error abriendo el fork\n");
                exit(EXIT_FAILURE);
            }
            else if(pid){
                pause();            }
            else{
                kill(getppid(), SIGUSR2);
            }


        }
    }

    while(1)
        pause(); /* Bloquea el proceso hasta que llegue una señal. */
}
