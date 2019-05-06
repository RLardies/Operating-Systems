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

int simpipe, id = -1, shmfd = -1;
int pipes[N_NAVES][2] = { -1 };
sem_t *sem_ini = NULL;
tipo_mapa *mapa = NULL;

void manejador_SIGTERM(int sig) {
	while (wait(NULL) > 0);
	for (int i = 0; i < N_NAVES; i++) if (pipes[i][1] != -1) close(pipes[i][1]);
	close(simpipe);
	if (mapa != NULL) munmap(mapa, sizeof(tipo_mapa));
	if (shmfd != -1) close(shmfd);
	if (sem_ini != NULL) sem_close(sem_ini);
	exit(EXIT_SUCCESS);
}

void crear_naves() {

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
	char buf[MAXMSGSIZE], buf2[20];
	int auxid;

	sscanf(argv[1], "%d %d", &simpipe, &id);

	act.sa_handler = manejador_SIGTERM;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("Error en sigaction jefe");
		kill(0, SIGTERM);
	}
	if ((sem_ini = sem_open(SEM_INICIO, O_RDWR)) == SEM_FAILED) {
		perror("Error abriendo semáforo jefe");
		kill(0, SIGTERM);
	}	
	if ((shmfd = shm_open(SHM_MAP_NAME, O_RDONLY, S_IRUSR)) < 0) {
		perror("Error abriendo memoria jefe");
		kill(0, SIGTERM);
	}
	if ((mapa = (tipo_mapa *) mmap(NULL, sizeof(tipo_mapa), PROT_READ, O_RDONLY, shmfd, 0)) == MAP_FAILED) {
		perror("Error en mmap jefe");
		kill(0, SIGTERM);
	}

	sem_wait(sem_ini);
	sem_post(sem_ini);

	while (1) {
		if (read(simpipe, buf, MAXMSGSIZE) < 0) {
			perror("Error recibiendo mensaje jefe");
			kill(0, SIGTERM);
		}	   
		if (strcmp(buf, TURNO) == 0) {
			while (mapa->info_naves[id][auxid = randint(0, N_NAVES)].viva == false);
			switch(randint(0, 2)) {
				case 0:
					if (write(pipes[auxid][1], ATACAR, sizeof(ATACAR)) < 0) {
						perror("Error enviando mensaje a nave");
						kill(0, SIGTERM);
					}
					break;
				case 1:
					if (write(pipes[auxid][1], MOVER_ALEATORIO, sizeof(MOVER_ALEATORIO)) < 0) {
						perror("Error enviando mensaje a nave");
						kill(0, SIGTERM);
					}
					break;
			}
		}
		else if (strcmp(buf, FIN) == 0) {
			for (int i = 0; i < N_NAVES; i++) {
				if (mapa->info_naves[id][i].viva == true) {
					if (write(pipes[i][1], DESTRUIR, sizeof(DESTRUIR)) < 0) {
						perror("Error mandando destruir a las naves");
						kill(0, SIGTERM);
					}
				}
			}
		}
		else {
			if (sscanf(buf, "%s %d", buf2, &auxid) < 0) {
				perror("Error obteniendo mensaje del simulador");
				kill(0, SIGTERM);
			}
			if (write(pipes[auxid][1], DESTRUIR, sizeof(DESTRUIR)) < 0) {
				perror("Error mandando destruir a las naves");
				kill(0, SIGTERM);
			}
		}
	}

	return EXIT_SUCCESS;
}
