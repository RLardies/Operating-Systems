#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5

int main(void){

	pid_t pid;
	int counter;
	sigset_t mask1, mask2;

	pid = fork();
	if(pid < 0){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0) {
		/*Creamos una máscara que contenga las señales a bloquear*/
		sigemptyset(&mask1);
		sigaddset(&mask1, SIGUSR1);
		sigaddset(&mask1, SIGUSR2);
		sigaddset(&mask1, SIGALRM);

		/*Creamos otra máscara que contenga las señales que se desbloquearán después*/
		sigemptyset(&mask2);
		sigaddset(&mask2, SIGUSR1);
		sigaddset(&mask2, SIGALRM);
		/*Programamos la alarma para dentro de 40 segundos*/
		alarm(40);

		while(1) {
			/*Ponemos la máscara que bloquea todas las señales para que no sea interrumpido
			 mientras cuenta*/
			sigprocmask(SIG_SETMASK, &mask1, NULL);

			for(counter = 0; counter < N_ITER; counter++) {
				printf("%d\n", counter);
				sleep(1);
			}
			/*Ahora se desbloqeuan las señales SIGALRM y SIGUSR1*/
			sigprocmask(SIG_UNBLOCK, &mask2, &mask1);
			sleep(3);
		}
	}
	while(wait(NULL) > 0);
}