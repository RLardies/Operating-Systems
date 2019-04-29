#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "mapa.h"

void manejador_SIGTERM(int sig) {

}

int main(int argc, char *argv[]) {

	int sim, i, j;
	int pipes[N_NAVES][2];
	struct sigaction act;
	pid_t pid;
	char buf[100];

	act.sa_handler = manejador_SIGTERM;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("Error en sigaction jefe");
		kill(0, SIGTERM);
	}

	for (i = 0; i < N_NAVES; i++) {
		if (pipe(pipes[i]) < 0) {
			perror("Error creando pipes jefe");
			kill(0, SIGTERM);
		}
		if ((pid = fork()) < 0) {
			perror("Error creando nave");
			kill(0, SIGTERM);
		}
		if (pid == 0) {
			for (j = 0; j < i; j++) {
				close(pipes[j][0]);
				close(pipes[j][1]);
			}
			close(pipes[i][1]);
			itoa(pipes[i][0], buf, 10);
			execl("nave", "nave", buf, (char *) NULL);
			perror("Error en exec");
			kill(0, SIGTERM);
		}	
		close(pipes[i][0]);
	}

	return EXIT_SUCCESS;
}
