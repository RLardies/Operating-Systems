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

int simpipe, id;
int pipes[N_NAVES][2];

void manejador_SIGTERM(int sig) {
	while (wait(NULL) > 0);
	for (int i = 0; i < N_NAVES; i++) close(pipes[i][1]);
	close(simpipe);
	exit(EXIT_SUCCESS);
}

void crear_naves(int pipes[N_NAVES][2]) {

	pid_t pid;
	char buf[100];
	int i, j; 

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
			sprintf(buf, "%d %d %d", pipes[i][0], i, id);
			execl("nave", "nave", buf, (char *) NULL);
			perror("Error en exec");
			kill(0, SIGTERM);
		}	
		close(pipes[i][0]);
	}
}

int main(int argc, char *argv[]) {

	struct sigaction act;

	sscanf(argv[1], "%d %d", &simpipe, &id);

	act.sa_handler = manejador_SIGTERM;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("Error en sigaction jefe");
		kill(0, SIGTERM);
	}

	return EXIT_SUCCESS;
}
