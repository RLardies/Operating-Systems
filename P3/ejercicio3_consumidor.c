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
#define FILE "/mi_fichero"
#define MAX_COLA 10

int main(int argc, char *argv[]) {

	int fd;
	char c;
	Queue *q;
	sem_t *sem;
	sem_t *sem_cola_llena;
	sem_t *sem_cola_vacia;

	if ((fd = shm_open(FILE, O_RDWR , S_IRUSR | S_IWUSR)) < 0) {
		perror("Error en shm_open");
		exit(EXIT_FAILURE);
	}
	
	if ((q = (Queue *) mmap(NULL, sizeof(q), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

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

	while (1) {
		sem_wait(sem_cola_vacia);
		sem_wait(sem);
<<<<<<< HEAD
		if (cola_extraer(q, &c) < 0) {
			perror("Error extrayendo de la cola\n");	
			sem_close(sem);
			sem_close(sem_cola_vacia);
			sem_close(sem_cola_llena);
			exit(EXIT_FAILURE);
		}
=======
		sem_wait(sem_cola_llena);

		cola_extraer(q,&c);

		if(c == '\0'){
			sem_unlink(SEM);
			sem_unlink(SEM_COLA_VACIA);
			sem_unlink(SEM_COLA_LLENA);
>>>>>>> 08452fcca45a95b69e0057136a03aff8040b5618

		if (c == '\0') {
			sem_post(sem);
			munmap(q,sizeof(Queue*));
			sem_close(sem);
			sem_close(sem_cola_vacia);
			sem_close(sem_cola_llena);
			exit(EXIT_SUCCESS);
		}
		else
			printf("Caracter extraido: %c\n", c);

		sem_post(sem_cola_llena);
		sem_post(sem);
	}

	return 0;
}
