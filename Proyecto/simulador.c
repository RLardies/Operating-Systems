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
sem_t *sem_inicio = NULL, *mutex = NULL, *sem_pantalla = NULL;
int pipes[N_EQUIPOS][2] = {{ -1 }};

void manejador_SIGTERM(int sig) {
	for (int i = 0; i < N_EQUIPOS; i++) {
		if (pipes[i][1] != -1) {
			write(pipes[i][1], FIN, sizeof(FIN));
			close(pipes[i][1]);
		}
	}
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
	if (mutex != NULL) {
		sem_unlink(SEM_MEMORIA);
		sem_close(mutex);
	}
	if (sem_pantalla != NULL) {
		sem_unlink(SEM_PANTALLA);
		sem_close(sem_pantalla);	
	}
	exit(EXIT_SUCCESS);
}

void ceder_turno() {
	for (int i = 0; i < N_EQUIPOS; i++) {
		if (write(pipes[i][1], TURNO, sizeof(TURNO)) < 0) {
			perror("Error enviando turno");
			raise(SIGTERM);
		}
	}
}

void manejador_SIGALRM(int sig) {
	int i, j = -1;
	sem_wait(mutex);
	mapa_restore(mapa);
	for (i = 0; i < N_EQUIPOS; i++) {
		if (mapa->num_naves[i] > 0) {
			if (j != -1) {
				sem_post(mutex);
				ceder_turno();
				alarm(5);
				return;
			}
			j = i;
		}
	}
	sem_wait(sem_pantalla);
	if (j == -1) printf("Simulador: Ha habido un empate!\n");
	else printf("El ganador es el equipo número %d!\n", j);
	sem_post(sem_pantalla);
	raise(SIGTERM);
}

void distribuir_naves(tipo_mapa *mapa) {
	int i, j, x, y;
	tipo_nave auxnave = {
		.vida = VIDA_MAX,
		.viva = true
	};

	sem_wait(mutex);
	mapa_clean(mapa);
	sem_post(mutex);

	for (i = 0; i < N_EQUIPOS; i++) {
		for (j = 0; j < N_NAVES; j++) {
			auxnave.equipo = i;
			auxnave.numNave = j;
			sem_wait(mutex);
			while (mapa_is_casilla_vacia(mapa, (y = randint(0, MAPA_MAXY)), 
				(x = randint(0, MAPA_MAXX))) == false);
			sem_post(mutex);
			auxnave.posx = x;
			auxnave.posy = y;
			sem_wait(mutex);
			mapa_set_nave(mapa, auxnave);
			sem_post(mutex);
		}
	}
}

void crear_jefes() {

	int i, j;
	pid_t pid;
	char buf[100];

	for (i = 0; i < N_EQUIPOS; i++) {
		sem_wait(mutex);
		mapa_set_num_naves(mapa, i, N_NAVES);
		sem_post(mutex);
		if (pipe(pipes[i]) < 0) {
			perror("Error creando pipes");
			kill(0, SIGTERM);
		}
		if ((pid = fork()) < 0) {
			perror("Error creando jefe");
			kill(0, SIGTERM);
		}
		if (pid == 0) {
			for (j = 0; j <= i; j++) close(pipes[j][1]);
			sprintf(buf, "%d %d", pipes[i][0], i);
			execl("jefe", "jefe", buf, (char *) NULL);
			perror("Error en exec");
			kill(0, SIGTERM);
		}	
		close(pipes[i][0]);
	}
}

int main() {

	int ret=0, i, j;
	tipo_mapa *mapa;
	char buf[100];
	struct sigaction act;
	sigset_t mask;
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
	act.sa_handler = manejador_SIGALRM;
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}
	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGALRM);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	if ((cola = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &qattr)) < 0) {
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
	if ((mapa = (tipo_mapa *) mmap(NULL, sizeof(tipo_mapa), PROT_READ | PROT_WRITE, 
		MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		kill(0, SIGTERM);
	}
	if ((sem_inicio = sem_open(SEM_INICIO, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
		perror("Error creando el semaforo");
		kill(0, SIGTERM);
	}
	if ((mutex = sem_open(SEM_MEMORIA, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("Error creando el semaforo");
		kill(0, SIGTERM);
	}
	if ((sem_pantalla= sem_open(SEM_PANTALLA, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("Error creando el semaforo");
		kill(0, SIGTERM);
	}

	crear_jefes();
	distribuir_naves(mapa);		

	for (i = 0; i < N_NAVES * N_EQUIPOS; i++) {
		if (mq_receive(cola, buf, MAXMSGSIZE, NULL) < 0) {
			perror("Error recibiendo mensaje inicial");
			kill(0, SIGTERM);
		}
		if (strcmp(buf, "READY")) {
			perror("Error con mensaje de inicio");
			kill(0, SIGTERM);
		}
	}
	sem_post(sem_inicio);

	while(1) {
		if (mq_receive(cola, buf, MAXMSGSIZE, NULL) < 0) {
			perror("Error recibiendo mensaje acción");
			kill(0, SIGTERM);
		}
		
	}
	
    exit(ret);
}
