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

mqd_t cola = -1;
int shmfd = -1;
tipo_mapa *mapa = NULL;
sem_t *sem_inicio = NULL;

void manejador_SIGTERM(int sig) {
	while (wait(NULL) > 0);
	if (cola > 0) {
		mq_unlink(QUEUE_NAME);
		mq_close(cola);
	}
	if (shmfd > 0) {
		shm_unlink(SHM_MAP_NAME);
		close(shmfd);
	}
	if (mapa != NULL) munmap(mapa, sizeof(tipo_mapa));
	if (sem_inicio > 0) {
		sem_unlink(SEM_INICIO);
		sem_close(sem_inicio);
	}
	exit(EXIT_SUCCESS);
}

int main() {

	int ret=0, i, j;
	int pipes[N_EQUIPOS][2];
	struct sigaction act;
	pid_t pid;
	char buf[100];
	struct mq_attr qattr = {
		.mq_flags = 0,
		.mq_maxmsg = 10,
		.mq_msgsize = MAXMSGSIZE,
		.mq_curmsgs = 0
	};

	act.sa_handler = manejador_SIGTERM;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}
	if ((cola = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error creando la cola");
		kill(0, SIGTERM);
	}
	if ((shmfd = shm_open(SHM_MAP_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error abriendo memoria compartida");
		kill(0, SIGTERM);
	}
	if (ftruncate(shmfd, sizeof(tipo_mapa)) < 0) {
		perror("Error en ftruncate");
		kill(0, SIGTERM);
	}
	if ((mapa = mmap(NULL, sizeof(tipo_mapa), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		kill(0, SIGTERM);
	}
	if ((sem_inicio = sem_open(SEM_INICIO, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
		perror("Error creando el semaforo");
		kill(0, SIGTERM);
	}

	for (i = 0; i < N_EQUIPOS; i++) {
		if (pipe(pipes[i]) < 0) {
			perror("Error creando pipes");
			kill(0, SIGTERM);
		}
		if ((pid = fork()) < 0) {
			perror("Error creando jefe");
			kill(0, SIGTERM);
		}
		if (pid == 0) {
			for (j = 0; j < i; j++) {
				close(pipes[j][0]);
				close(pipes[j][1]);
			}
			close(pipes[i][1]);
			itoa(pipes[i][0], buf, 10);
			execl("jefe", "jefe", buf, (char *) NULL);
			perror("Error en exec");
			kill(0, SIGTERM);
		}	
		close(pipes[i][0]);
	}
	
    exit(ret);
}



