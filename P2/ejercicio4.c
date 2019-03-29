#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_PROC 4

void manejador_corredor(int sig) {

	printf("Yo, el proceso %lld, he terminado.\n", (long long)getpid());
	exit(EXIT_SUCCESS);
}

void manejador_gestor(int sig) {
	
}

void manejador_padre(int sig) {
	/*Cuando el padre reciba la señal SIGUSR2 mandara la señal SIGUSR1 a todo el grupo
	 pero tanto él como el gestor la ignorarán*/
	if (kill(0, SIGUSR1) < 0) {
		perror("Error en kill");
		exit(EXIT_FAILURE);
	}
}

int main () {

	struct sigaction act, act2;
	pid_t pid, pid2;
	int i;
	/*Actuador del padre para la señal SIGUSR2*/
	act.sa_handler = manejador_padre;
	act.sa_flags = 0;
	if (sigaction(SIGUSR2, &act, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) < 0) {
		perror("Error creando el gestor");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		/*Actuador del gestor para la señal SIGUSR2*/
		act.sa_handler = manejador_gestor;
		act.sa_flags = 0;
		if (sigaction(SIGUSR2, &act, NULL) < 0) {
			perror("Error en sigaction");
			exit(EXIT_FAILURE);
		}
		/*Hacemos que el gestor ignore la señal SIGUSR1*/
		act2.sa_handler = SIG_IGN;
		act2.sa_flags = 0;
		if (sigaction(SIGUSR1, &act2, NULL)) {
			perror("Error en sigaction");
			exit(EXIT_FAILURE);
		}
		/*Creamos los hijos esperando cada vez a que estén listos*/
		for (i = 0; i < N_PROC; i++) {
			if ((pid2 = fork()) < 0) {
				perror("Error creando uno de los participantes");
				exit(EXIT_FAILURE);
			}
			if (pid2 == 0) {
				act.sa_handler = manejador_corredor;
				act.sa_flags = 0;
				if (sigaction(SIGUSR1, &act, NULL) < 0) {
					perror("Error en sigaction");
					exit(EXIT_FAILURE);
				}
				printf("El proceso %lld está listo\n", (long long) getpid());
				kill(getppid(), SIGUSR2);
			}
			pause();
		}
		/*Cuando estan todos listos, le manda la señal al padre*/
		if (kill(getppid(), SIGUSR2) < 0) {
			perror("Error en kill");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < N_PROC; i++) wait(NULL);
		exit(EXIT_SUCCESS);

	}
	/*Hacemos que el padre ignore la señal SIGUSR1*/
	act2.sa_handler = SIG_IGN;
	act2.sa_flags = 0;
	if (sigaction(SIGUSR1, &act2, NULL)) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}

	pause();
	/*Tras la señal esperará a que acabe el gestor, después de acabar los participantes*/
	wait(NULL);
	return EXIT_SUCCESS;
}