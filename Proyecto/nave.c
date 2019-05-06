#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "mapa.h"

int pipejefe, shmfd = -1, idjefe, id;
mqd_t cola = -1;
tipo_mapa *mapa = NULL;
sem_t *sem_ini = NULL, *mutex = NULL, *sem_pantalla = NULL;

void manejador_SIGUSR1(int sig) {
	close(pipejefe);
	if (cola != -1) mq_close(cola);
	if (mapa == NULL) munmap(mapa, sizeof(tipo_mapa));
	if (sem_ini != NULL) sem_close(sem_ini);
	exit(EXIT_SUCCESS);
}

/*
 * Busca alguna nave que este a rango de disparo (coord son las coordenadas de la nave y
 * donde se gusrdará el resultado de la búsqueda). Solo hay una cierta probabilidad de que
 * la encuentre, y de no ser así disparará al agua
 */
void buscar_nave(tipo_mapa *mapa, int coord[2]) {
	int i, x = coord[1], y = coord[0];
	
	for (i = 0; i < ATAQUE_ALCANCE * ATAQUE_ALCANCE / 5; i++) {
		sem_wait(mutex);
		while (mapa_get_distancia(mapa, coord[0] = randint(0, MAPA_MAXY), coord[1] = randint(0, MAPA_MAXX), y, x) > ATAQUE_ALCANCE 
				|| mapa_get_casilla(mapa, coord[0], coord[1]).equipo == idjefe);
		if (!mapa_is_casilla_vacia(mapa, y, x)) return;
		sem_post(mutex);
	}
}
/*
 * Manda al simulador una nave enemiga que se elige con la función buscar_nave para que la ataque
 */
void atacar_nave() {
	int coord[2];
	char buf[MAXMSGSIZE];
	
	sem_wait(mutex);
	coord[0] = mapa->info_naves[idjefe][id].posy;
	coord[1] = mapa->info_naves[idjefe][id].posx;
	sem_post(mutex);
	buscar_nave(mapa, coord);
	sprintf(buf, "ATACAR %d %d -1 -1", coord[0], coord[1]);
	if (mq_send(cola, buf, MAXMSGSIZE, 1) < 0) {
		perror("Error enviando ataque nave");
		raise(SIGUSR1);
	}
}

/*
 * Manda un mensaje al simulador para que se mueva en una dirección aleatoria en su rango
 * de movimiento.
 */
void mover_nave() {
	sem_wait(mutex);
	int x = mapa->info_naves[idjefe][id].posx, y = mapa->info_naves[idjefe][id].posy; 
	char buf[MAXMSGSIZE];

	do {
		switch(randint(0, 4)) {
			case 0:
				y += randint(1, MOVER_ALCANCE + 1); break;
			case 1:
				x += randint(1, MOVER_ALCANCE + 1); break;
			case 2:
				y -= randint(1, MOVER_ALCANCE + 1); break;
			case 3:
				x -= randint(1, MOVER_ALCANCE + 1); break;
		}
	} while(x < 0 || y < 0 || x > MAPA_MAXX || y > MAPA_MAXY);	

	sprintf(buf, "MOVER %d %d %d %d", mapa->info_naves[idjefe][id].posy, mapa->info_naves[idjefe][id].posx, 
		y, x);	
	sem_post(mutex);
	if (mq_send(cola, buf, MAXMSGSIZE, 1) < 0) {
		perror("Error enviando movimiento nave");
		kill(0, SIGTERM);
		raise(SIGUSR1);
	}
}

void destruir_nave() {
	sem_wait(mutex);
	mapa->info_naves[idjefe][id].viva = false;
	mapa->num_naves[idjefe]--;
	sem_post(mutex);
	raise(SIGUSR1);
}

int main (int argc, char *argv[]) {

	char buf[MAXMSGSIZE];
	struct sigaction act = {
		.sa_handler = manejador_SIGUSR1,
		.sa_flags = 0
	};
	sscanf(argv[1], "%d %d %d", &pipejefe, &id, &idjefe);
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror("Error en sigaction nave");
		destruir_nave();
	}

	if ((cola = mq_open(QUEUE_NAME, O_WRONLY)) < 0) {
		perror("Error abriendo cola nave");
		destruir_nave();
	}	
	if ((shmfd = shm_open(SHM_MAP_NAME, O_RDONLY, S_IRUSR)) < 0) {
		perror("Error abriendo memoria nave");
		destruir_nave();
	}
	if ((mapa = (tipo_mapa *) mmap(NULL, sizeof(tipo_mapa), PROT_READ, MAP_PRIVATE, shmfd, 0)) == MAP_FAILED) {
		perror("Error en mmap nave");
		destruir_nave();
	}
	if ((sem_ini = sem_open(SEM_INICIO, O_RDWR)) == SEM_FAILED) {
		perror("Error abriendo semáforo nave");
		destruir_nave();
	}
	if ((mutex = sem_open(SEM_MEMORIA, O_RDWR)) == SEM_FAILED) {
		perror("Error abriendo semáforo nave");
		destruir_nave();
	}
	if ((sem_pantalla= sem_open(SEM_PANTALLA, O_RDWR)) == SEM_FAILED) {
		perror("Error abriendo semáforo nave");
		destruir_nave();
	}

	if (mq_send(cola, "READY", sizeof("READY"), 1) < 0) {
		perror("Error enviando mensaje nave");
		destruir_nave();
	}

	sem_wait(sem_ini);
	sem_post(sem_ini);

	while(1) {
		if (read(pipejefe, buf, MAXMSGSIZE) < 0) {
			perror("Error en recepcion de acción nave");
			kill(0, SIGTERM);
			raise(SIGUSR1);
		}
		if (strcmp(buf, ATACAR) == 0) atacar_nave();
		else if (strcmp(buf, MOVER_ALEATORIO) == 0) mover_nave();
		else if (strcmp(buf, DESTRUIR) == 0) destruir_nave();
		else {
			perror("Error nave inválido");
			destruir_nave();
		}
	}
}
