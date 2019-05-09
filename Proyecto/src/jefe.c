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
#include <time.h>

#include "mapa.h"

static int simpipe, id = -1, shmfd = -1;
static int pipes[N_NAVES][2] = {{ -1 }};
static sem_t *sem_ini = NULL, *mutex = NULL, *sem_pantalla = NULL;
static tipo_mapa *mapa = NULL;

/**
 * @brief      Manejador de la señal SIGUSR2. Se encarga de mandar finalizar a todas sus naves,
 *             esperarlas, liberar los recursos utilizados y terminar.
 *
 * @param[in]  sig   Señal
 */
void manejador_SIGUSR2(int sig) {

	sem_wait(mutex);
	for (int i = 0; i < N_NAVES; i++) {
		if (mapa->info_naves[id][i].viva == true) {
			write(pipes[i][1], DESTRUIR, sizeof(DESTRUIR));
			close(pipes[i][1]);				
		}
	}
	sem_post(mutex);
	while (wait(NULL) > 0);
	close(simpipe);
	if (mapa != NULL) munmap(mapa, sizeof(tipo_mapa));
	if (shmfd != -1) close(shmfd);
	if (sem_ini != NULL) sem_close(sem_ini);
	exit(EXIT_SUCCESS);
}

/**
 * @brief      Crea las naves de su equipo y los pipes con los que se va a comunicar con ellas.
 */
void crear_naves() {

	pid_t pid;
	char buf[100];
	int i, j; 

	sem_wait(sem_pantalla);
	printf("Jefe %c : creando naves\n", symbol_equipos[id]);
	sem_post(sem_pantalla);

	for (i = 0; i < N_NAVES; i++) {
		if (pipe(pipes[i]) < 0) {
			perror("Error creando pipes jefe");
			raise(SIGUSR2);
		}
		if ((pid = fork()) < 0) {
			perror("Error creando nave");
			raise(SIGUSR2);
		}
		if (pid == 0) {
			for (j = 0; j <= i; j++) close(pipes[j][1]);
			sprintf(buf, "%d %d %d", pipes[i][0], i, id);
			execl("nave", "nave", buf, (char *) NULL);
			perror("Error en exec");
			raise(SIGUSR2);
		}
		sem_wait(sem_pantalla);
		printf("Jefe %c : Creada nave %d\n", symbol_equipos[id], i);
		sem_post(sem_pantalla);
		close(pipes[i][0]);
	}
}

int main(int argc, char *argv[]) {

	struct sigaction act;
	char buf[MAXMSGSIZE], buf2[20];
	int auxid, sizeatacar = sizeof(ATACAR), sizemover = sizeof(MOVER_ALEATORIO);
	int sizedestruir = sizeof(DESTRUIR);
	sigset_t mask;

	sscanf(argv[1], "%d %d", &simpipe, &id);
	sigprocmask(0, NULL, &mask);
	sigaddset(&mask, SIGTERM);
	sigdelset(&mask, SIGUSR2);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	act.sa_handler = manejador_SIGUSR2;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("Error en sigaction jefe");
		raise(SIGUSR2);
	}
	if ((sem_ini = sem_open(SEM_INICIO, O_RDWR)) == SEM_FAILED) {
		perror("Error abriendo semáforo jefe");
		raise(SIGUSR2);
	}	
	if ((mutex = sem_open(SEM_MEMORIA, O_RDWR)) == SEM_FAILED) {
		perror("Error abriendo semáforo jefe");
		raise(SIGUSR2);
	}	
	if ((sem_pantalla = sem_open(SEM_PANTALLA, O_RDWR)) == SEM_FAILED) {
		perror("Error abriendo semáforo jefe");
		raise(SIGUSR2);
	}
	sem_wait(sem_pantalla);
	printf("Jefe %c abriendo memoria compartida\n", symbol_equipos[id]);
	sem_post(sem_pantalla);
	if ((shmfd = shm_open(SHM_MAP_NAME, O_RDONLY, S_IRUSR)) < 0) {
		perror("Error abriendo memoria jefe");
		raise(SIGUSR2);
	}
	sem_wait(sem_pantalla);
	printf("Jefe %c inicializando mapa\n", symbol_equipos[id]);
	sem_post(sem_pantalla);
	if ((mapa = (tipo_mapa *) mmap(NULL, sizeof(tipo_mapa), PROT_READ, MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
		perror("Error en mmap jefe");
		raise(SIGUSR2);
	}
	srand(time(NULL) + getpid());
	crear_naves();

	sem_wait(sem_ini);
	sem_post(sem_ini);

	while (1) {
		if (read(simpipe, buf, MAXMSGSIZE) < 0) {
			perror("Error recibiendo mensaje jefe");
			raise(SIGUSR2);
		}	   
		if (strcmp(buf, TURNO) == 0) {
			for (int i = 0; i < N_ACCIONES; i++) {
				sem_wait(mutex);
				while (mapa->info_naves[id][auxid = randint(0, N_NAVES)].viva == false);
				sem_post(mutex);
				/**
				 * Enviamos dos mensajes al pipe, uno con el tamaño del mensaje, y otro con
				 * el mensaje en sí.
				 */
				switch(randint(0, 2)) {
					case 0:
						if (write(pipes[auxid][1], &sizeatacar, sizeof(int)) < 0) {
							perror("Error enviando mensaje a nave");
							raise(SIGUSR2);
						}
						if (write(pipes[auxid][1], ATACAR, sizeatacar) < 0) {
							perror("Error enviando mensaje a nave");
							raise(SIGUSR2);
						}
						break;
					case 1:
						if (write(pipes[auxid][1], &sizemover, sizeof(int)) < 0) {
							perror("Error enviando mensaje a nave");
							raise(SIGUSR2);
						}
						if (write(pipes[auxid][1], MOVER_ALEATORIO, sizemover) < 0) {
							perror("Error enviando mensaje a nave");
							raise(SIGUSR2);
						}
						break;
					default:
						printf("Error al generar número aleatroio\n");
				}
			}
		}
		else if (strcmp(buf, FIN) == 0) raise(SIGUSR2);
		else {
			if (sscanf(buf, "%s %d", buf2, &auxid) < 0) {
				perror("Error obteniendo mensaje del simulador");
				raise(SIGUSR2);
			}
			if (strcmp(buf2, DESTRUIR)) {
				if (write(pipes[auxid][1], &sizedestruir, sizeof(int)) < 0) {
						perror("Error enviando mensaje a nave");
						raise(SIGUSR2);
					}
				if (write(pipes[auxid][1], DESTRUIR, sizedestruir) < 0) {
					perror("Error mandando destruir a las naves");
					raise(SIGUSR2);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
