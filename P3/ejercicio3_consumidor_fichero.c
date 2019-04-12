#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include "cola.h"

#define SEM "/sem"
#define SEM_COLA_LLENA "/sem_cola_llena"
#define SEM_COLA_VACIA "/sem_cola_vacia"
#define FILE "mi_fichero"
#define MAX_COLA 10

int main(int argc, char *argv[]) {

	int fd;
	char c;
	Queue *q;
	sem_t *sem;
	sem_t *sem_cola_llena;
	sem_t *sem_cola_vacia;

	/*Abrimos la región de memoria compartida ya creada*/
	if ((fd = open(FILE, O_RDWR , S_IRUSR | S_IWUSR)) < 0) {
		perror("Error en shm_open");
		exit(EXIT_FAILURE);
	}
	
	if ((q = (Queue *) mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

	/*Abrimos los semáforos que vamos a utilizar también creados anteriormente*/
    if ((sem = sem_open(SEM, O_CREAT)) == SEM_FAILED) {
        printf("Error abriendo semaforo\n");
		shm_unlink(FILE);
        exit(EXIT_FAILURE);
    }

    if ((sem_cola_vacia = sem_open(SEM_COLA_VACIA, 0)) == SEM_FAILED) {
        printf("Error abriendo semaforo\n");
		shm_unlink(FILE);
		sem_unlink(SEM);
        exit(EXIT_FAILURE);
    }

    if ((sem_cola_llena = sem_open(SEM_COLA_LLENA, 0)) == SEM_FAILED) {
        printf("Error abriendo semaforo\n");
		shm_unlink(FILE);
		sem_unlink(SEM);
		sem_unlink(SEM_COLA_VACIA);
        exit(EXIT_FAILURE);
    }

	/*Comprobamos constantemente que haya algo en la cola y lo extraemos protegiéndola
	 * antes*/
	while (1) {
		sem_wait(sem_cola_vacia);
		sem_wait(sem);
		if (cola_extraer(q, &c) < 0) {
			perror("Error extrayendo de la cola\n");	
			sem_close(sem);
			sem_close(sem_cola_vacia);
			sem_close(sem_cola_llena);
			exit(EXIT_FAILURE);
		}

		/*Si el caracter es el '\0' finalizamos el programa*/
		if (c == '\0') {
			printf("\n");
			sem_post(sem);
			munmap(q,sizeof(Queue*));
			sem_close(sem);
			sem_close(sem_cola_vacia);
			sem_close(sem_cola_llena);
			exit(EXIT_SUCCESS);
		}
		/*Si no lo es lo imprimimos*/
		else
			printf("Caracter extraido: %c\n", c);

		sem_post(sem_cola_llena);
		sem_post(sem);
	}

	return 0;
}
