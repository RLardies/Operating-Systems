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
	mapa->finalizado = true;
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
	sem_wait(sem_pantalla);
	printf("\n\nSimulador : nuevo TURNO\n");
	sem_post(sem_pantalla);
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
	sem_post(mutex);
	sem_wait(sem_pantalla);
	if (j == -1) printf("Simulador: Ha habido un empate!\n");
	else printf("**** EQUIPO GANADOR %c *******\n", symbol_equipos[j]);
	sem_post(sem_pantalla);
	raise(SIGTERM);
}

void distribuir_naves() {
	int i, j, x, y;
	tipo_nave auxnave = {
		.vida = VIDA_MAX,
		.viva = true
	};

	sem_wait(sem_pantalla);
	printf("Simulador : distribuyendo naves\n");
	sem_post(sem_pantalla);
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

	printf("Simulador creando jefes de equipo\n");
	for (i = 0; i < N_EQUIPOS; i++) {
		sem_wait(mutex);
		mapa_set_num_naves(mapa, i, N_NAVES);
		sem_post(mutex);
		if (pipe(pipes[i]) < 0) {
			perror("Error creando pipes");
			raise(SIGTERM);
		}
		if ((pid = fork()) < 0) {
			perror("Error creando jefe");
			raise(SIGTERM);
		}
		if (pid == 0) {
			for (j = 0; j <= i; j++) close(pipes[j][1]);
			sprintf(buf, "%d %d", pipes[i][0], i);
			execl("jefe", "jefe", buf, (char *) NULL);
			perror("Error en exec");
			raise(SIGTERM);
		}	
		close(pipes[i][0]);
		sem_wait(sem_pantalla);
		printf("Simulador : Creado jefe %c\n", symbol_equipos[i]);
		sem_post(sem_pantalla);
	}
}

void accion_mover(int oriy, int orix, int desy, int desx) {
	sem_wait(sem_pantalla);
	printf("ACCION MOVER [%c%d] %d,%d -> %d,%d", 
		mapa->casillas[oriy][orix].simbolo, mapa->casillas[oriy][orix].numNave, 
		oriy, orix, desy, desx);
	sem_post(sem_pantalla);
	sem_wait(mutex);
	if (!mapa_is_casilla_vacia(mapa, desy, desx)) {
		sem_post(mutex);
		sem_wait(sem_pantalla);
		printf(": FALLIDO : Casilla llena");
		sem_post(sem_pantalla);
		return;
	}
	mapa->casillas[desy][desx].simbolo = 
		mapa->casillas[oriy][orix].simbolo;
	mapa->casillas[desy][desx].numNave = 
		mapa->casillas[oriy][orix].numNave;
	mapa->casillas[desy][desx].equipo = 
		mapa->casillas[oriy][orix].equipo;
	mapa_clean_casilla(mapa, oriy, orix);
	sem_post(mutex);
	sem_wait(sem_pantalla);
	printf("\n");
	sem_post(sem_pantalla);
}

void accion_atacar(int oriy, int orix, int desy, int desx) {
	char buf[MAXMSGSIZE];

	sem_wait(sem_pantalla);
	printf("ACCION ATAQUE [%c%d] %d,%d -> %d,%d", 
		mapa->casillas[oriy][orix].simbolo, mapa->casillas[oriy][orix].numNave, 
		oriy, orix, desy, desx);
	sem_post(sem_pantalla);
	sem_wait(mutex);
	mapa_send_misil(mapa, oriy, orix, desy, desx);
	if (mapa_is_casilla_vacia(mapa, desy, desx)) {
		sem_wait(sem_pantalla);
		printf(": FALLIDO : Casilla target vacía\n");
		sem_post(sem_pantalla);
		sem_post(mutex);
		return;
	}
	mapa->info_naves[mapa->casillas[desy][desx].equipo][mapa->casillas[desy][desx]
		.numNave].vida -= ATAQUE_DANO;
	if (mapa->info_naves[mapa->casillas[desy][desx].equipo][mapa->casillas[desy][desx]
		.numNave].vida <= 0) {
		sprintf(buf, DESTRUIR "%d", mapa->casillas[desy][desx].numNave);
		if (write(pipes[mapa->casillas[desy][desx].equipo][1], buf, MAXMSGSIZE) < 0) {
			perror("Error enviando destrucción al jefe");
			sem_post(mutex);
			raise(SIGTERM);
		}
		sem_wait(sem_pantalla);
		printf(": target destruido\n");
		sem_post(sem_pantalla);
		sem_post(mutex);
		return;
	}
	sem_post(mutex);
	sem_wait(sem_pantalla);
	printf(": target a %d de vida\n", mapa->info_naves[mapa->casillas[desy][desx]
		.equipo][mapa->casillas[desy][desx].numNave].vida);
	sem_post(sem_pantalla);
}

int main() {

	int i, x1, x2, x3, x4, naves_vivas;
	char buf[MAXMSGSIZE], buf2[30];
	struct sigaction act;
	sigset_t mask;

	struct mq_attr qattr = {
		.mq_flags = 0,
		.mq_maxmsg = 10,
		.mq_msgsize = MAXMSGSIZE,
		.mq_curmsgs = 0
	};
	printf("Simulador estableciendo máscara y manejadores\n");
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
	sigprocmask(SIG_SETMASK, &mask, NULL);
	sigfillset(&mask);
	sigdelset(&mask, SIGALRM);

	printf("Simulador creando cola de mensajes\n");
	if ((cola = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &qattr)) < 0) {
		perror("Error creando la cola");
		raise(SIGTERM);
	}
	printf("Simulador creando memoria compartida\n");
	if ((shmfd = shm_open(SHM_MAP_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error abriendo memoria compartida");
		raise(SIGTERM);
	}
	if (ftruncate(shmfd, sizeof(tipo_mapa)) < 0) {
		perror("Error en ftruncate");
		raise(SIGTERM);
	}
	printf("Simulador inicializando mapa\n");
	if ((mapa = (tipo_mapa *) mmap(NULL, sizeof(tipo_mapa), PROT_READ | PROT_WRITE, 
		MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		raise(SIGTERM);
	}
	printf("Simulador inicializando semáforos\n");
	if ((sem_inicio = sem_open(SEM_INICIO, O_CREAT , S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
		perror("Error creando el semaforo");
		raise(SIGTERM);
	}
	if ((mutex = sem_open(SEM_MEMORIA, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("Error creando el semaforo");
		raise(SIGTERM);
	}
	if ((sem_pantalla= sem_open(SEM_PANTALLA, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("Error creando el semaforo");
		raise(SIGTERM);
	}

	crear_jefes();
	distribuir_naves();

	sem_wait(sem_pantalla);
	printf("Simulador : a la espera de las naves\n");
	sem_post(sem_pantalla);
	for (i = 0; i < N_NAVES * N_EQUIPOS; i++) {
		if (mq_receive(cola, buf, MAXMSGSIZE, NULL) < 0) {
			perror("Error recibiendo mensaje inicial");
			raise(SIGTERM);
			}
		if (strcmp(buf, "READY")) {
			printf("Error en el mensaje inicial\n");
			raise(SIGTERM);
		}
		printf("Simulador : %d naves preparadas\n", i + 1);
	}
	sem_unlink(SEM_INICIO);
	sem_unlink(SEM_MEMORIA);
	sem_unlink(SEM_PANTALLA);
	mq_unlink(QUEUE_NAME);
	shm_unlink(SHM_MAP_NAME);
	mapa->finalizado = false;
	sem_post(sem_inicio);
	raise(SIGALRM);
	sigsuspend(&mask);

	while(1) {
		sem_wait(mutex);
		for (i = 0, naves_vivas = 0; i < N_EQUIPOS; i++) 
			naves_vivas += mapa->num_naves[i];
		sem_post(mutex);
		sem_wait(sem_pantalla);
		printf("Simulador : escuchando cola de mensajes\n");
		sem_post(sem_pantalla);

		for (i = 0; i < naves_vivas; i++) {
			if (mq_receive(cola, buf, MAXMSGSIZE, NULL) < 0) {
				perror("Error recibiendo mensaje acción");
				raise(SIGTERM);
			}
		
			sem_wait(sem_pantalla);
			printf("Simulador : recibido en cola de mensajes\n");
			sem_post(sem_pantalla);
			sscanf(buf, "%s %d %d %d %d", buf2, &x1, &x2, &x3, &x4);

			if (strcmp(buf2, "MOVER") == 0) {
				accion_mover(x1, x2, x3, x4);
			}
			else if (strcmp(buf2, "ATACAR") == 0) {
				accion_atacar(x1, x2, x3, x4);
			}
			else {
				sem_wait(sem_pantalla);
				printf("%s\n", buf2);
				perror("Error en mensaje de acción");
				raise(SIGTERM);
			}
		}
		sigsuspend(&mask);
	}
}
