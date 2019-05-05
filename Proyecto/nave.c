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
tipo_mapa *mapa;

void manejador_SIGACTION(int sig) {
	close(pipejefe);
	if (cola != -1) mq_close(cola);
	if (mapa == NULL) munmap(mapa, sizeof(tipo_mapa));
	exit(EXIT_SUCCESS);
}

/*
 * Busca alguna nave que este a rango de disparo (coord son las coordenadas de la nave y
 * donde se gusrdará el resultado de la búsqueda)
 */
void buscar_nave(tipo_mapa *mapa, int coord[2]) {
	int i, x = coord[1], y = coord[0];
	
	for (i = 0; i < ATAQUE_ALCANCE * ATAQUE_ALCANCE / 5; i++) {
		while (mapa_get_distancia(mapa, coord[0] = randint(0, MAPA_MAXY), coord[1] = randint(0, MAPA_MAXX), y, x) > ATAQUE_ALCANCE 
				|| mapa_get_casilla(mapa, coord[0], coord[1]).equipo == idjefe);
		if (!mapa_is_casilla_vacia(mapa, y, x)) return;
	}
}

void atacar_nave() {
	int coord[2];
	char buf[MAXMSGSIZE];
	
	coord[0] = mapa->info_naves[idjefe][id].posy;
	coord[1] = mapa->info_naves[idjefe][id].posx;
	buscar_nave(mapa, coord);
	sprintf(buf, "ATACAR %d, %d", coord[0], coord[1]);
	if (mq_send(cola, buf, MAXMSGSIZE, 1) < 0) {
		perror("Error enviando ataque nave");
		kill(0, SIGTERM);
	}
}

void mover_nave() {
	int x = mapa->info_naves[idjefe][id].posx, y = mapa->info_naves[idjefe][id].posy; 
	char buf[MAXMSGSIZE];

	do {
		switch(randint(0, 4)) {
			case 0:
				y++; break;
			case 1:
				x++; break;
			case 2:
				y--; break;
			case 3:
				x--; break;
		}
	} while(x < 0 || y < 0 || x > MAPA_MAXX || y > MAPA_MAXY || !mapa_is_casilla_vacia(mapa, x, y));	

	sprintf(buf, "MOVER %d %d %d %d", mapa->info_naves[idjefe][id].posy, mapa->info_naves[idjefe][id].posx, 
		y, x);	
	if (mq_send(cola, buf, MAXMSGSIZE, 1) < 0) {
		perror("Error enviando movimiento nave");
		kill(0, SIGTERM);
	}
}

void destruir_nave() {
	mapa->info_naves[idjefe][id].viva = false;
	raise(SIGTERM);
}

int main (int argc, char *argv[]) {

	char buf[MAXMSGSIZE];
	struct sigaction act = {
		.sa_handler = manejador_SIGACTION,
		.sa_flags = 0
	};
	sscanf(argv[1], "%d %d %d", &pipejefe, &id, &idjefe);
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("Error en sigaction nave");
		kill(0, SIGTERM);
	}

	if ((cola = mq_open(QUEUE_NAME, O_WRONLY)) < 0) {
		perror("Error abriendo cola nave");
		kill(0, SIGTERM);
	}	
	if ((shmfd = shm_open(SHM_MAP_NAME, O_RDONLY, S_IRUSR)) < 0) {
		perror("Error abriendo memoria nave");
		kill(0, SIGTERM);
	}
	if ((mapa = (tipo_mapa *) mmap(NULL, sizeof(tipo_mapa), PROT_READ, MAP_PRIVATE, shmfd, 0)) == MAP_FAILED) {
		perror("Error en mmap nave");
		kill(0, SIGTERM);
	}

	if (mq_send(cola, "READY", sizeof("READY"), 1) < 0) {
		perror("Error enviando mensaje nave");
		kill(0, SIGTERM);
	}

	while(1) {
		if (read(pipejefe, buf, MAXMSGSIZE) < 0) {
			perror("Error en recepcion de acción nave");
			kill(0, SIGTERM);
		}
		if (strcmp(buf, ATACAR) == 0) atacar_nave();
		else if (strcmp(buf, MOVER_ALEATORIO) == 0) mover_nave();
		else if (strcmp(buf, DESTRUIR) == 0) destruir_nave();
		else {
			perror("Error nave inválido");
			kill(0, SIGTERM);
		}
	}

	return EXIT_SUCCESS;
}
