#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5

void manejador_SIGTERM(int sig) {

	printf("Soy %lld y he recibido la señal SIGTERM.\n", (long long) getpid());
	fflush(stdout);
	exit(EXIT_SUCCESS);
}

void manejador_SIGALRM(int sig) {
}

int main(void){

	pid_t pid;
	int counter;
	struct sigaction act;
	sigset_t mask;

	/*Establecemos una máscara de señales que bloquee todas las señales (bloqueables)*/
	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	pid = fork();
	if(pid < 0){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0) {
		/*Establecemos el manejador de SIGTERM*/
		act.sa_handler = manejador_SIGTERM;
		if (sigaction(SIGTERM, &act, NULL) < 0) {
			perror("Error en sigaction");
			exit(EXIT_FAILURE);
		}
		
		while(1) {
			/*Hace la cuenta, sin poder ser interrumpido*/
			for(counter = 0; counter < N_ITER; counter++) {
				printf("%d\n", counter);
				sleep(1);
			}

			/*Desbloquea la señal SIGTERM*/
			sigdelset(&mask, SIGTERM);
			sigprocmask(SIG_SETMASK, &mask, NULL);

			sleep(3);

			/*Vuleve a bloquearla*/
			sigaddset(&mask, SIGTERM);
			sigprocmask(SIG_SETMASK, &mask, NULL);
		}
	}
	/*Establece el manejador de la señal SIGALRM*/
	act.sa_handler = manejador_SIGALRM;
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}
	/*Desbloquea la señal SIGALRM*/
	sigdelset(&mask, SIGALRM);

	/*Programa la alarma para dentro de 40 segundos*/
	alarm(40);
	/*Espera hasta que llegue la señal SIGALRM (única no bloqueada)*/
	sigsuspend(&mask);

	/*Cuando llegue la señal mandará la señal SIGTERM al proceso que cuenta
	 y cuando acabe de contar, terminará*/
	kill(pid, SIGTERM);

	while(wait(NULL) > 0);
}