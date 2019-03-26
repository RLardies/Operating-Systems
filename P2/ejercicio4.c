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

	if (kill(0, SIGUSR1) < 0) {
		perror("Error en kill");
		exit(EXIT_FAILURE);
	}
}

int main () {

	struct sigaction act, act2;
	pid_t pid, pid2;
	int i;

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
	else if (pid == 0) {
		
		act.sa_handler = manejador_gestor;
		act.sa_flags = 0;
		if (sigaction(SIGUSR2, &act, NULL) < 0) {
			perror("Error en sigaction");
			exit(EXIT_FAILURE);
		}

		act2.sa_handler = SIG_IGN;
		act2.sa_flags = 0;
		if (sigaction(SIGUSR1, &act2, NULL)) {
			perror("Error en sigaction");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < N_PROC; i++) {
			if ((pid2 = fork()) < 0) {
				perror("Error creando uno de los participantes");
				exit(EXIT_FAILURE);
			}
			else if (pid2 == 0) {

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

		if (kill(getppid(), SIGUSR2) < 0) {
			perror("Error en kill");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < N_PROC; i++) wait(NULL);
		exit(EXIT_SUCCESS);

	}
	else{
		act2.sa_handler = SIG_IGN;
		act2.sa_flags = 0;
		if (sigaction(SIGUSR1, &act2, NULL)) {
			perror("Error en sigaction");
			exit(EXIT_FAILURE);
		}

		pause();

		wait(NULL);
	}
	return EXIT_SUCCESS;
}