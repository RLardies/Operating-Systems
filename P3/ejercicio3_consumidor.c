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
#include "cola.h"

#define SEM "/sem"
#define SEM_COLA_LLENA "/sem_cola_llena"
#define SEM_COLA_VACIA "/sem_cola_vacia"
#define FILE "/mi_fichero"
#define MAX_COLA 10


Queue *q;
sem_t *sem;
sem_t *sem_cola_llena;
sem_t *sem_cola_vacia;


int main(int argc, char *argv[]) {

	int fd;
	char c;

	if ((fd = open(FILE, O_RDWR , S_IRUSR | S_IWUSR)) < 0) {
		perror("Error en shm_open");
		exit(EXIT_FAILURE);
	}
	shm_unlink(FILE);
	
	if ((q = (Queue *) mmap(NULL, sizeof(q), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}

	sem = sem_open(SEM, 0);
    if(sem == SEM_FAILED) {
        printf("Error abriendo semaforo\n");
        exit(EXIT_FAILURE);
    }

    sem_cola_vacia = sem_open(SEM_COLA_VACIA, 0);
    if(sem_cola_vacia == SEM_FAILED) {
        printf("Error abriendo semaforo\n");
        exit(EXIT_FAILURE);
    }

    sem_cola_llena = sem_open(SEM_COLA_LLENA, 0);
    if(sem_cola_llena == SEM_FAILED) {
        printf("Error abriendo semaforo\n");
        exit(EXIT_FAILURE);
    }

	while(1){
		sem_wait(sem);
		sem_wait(sem_cola_llena);

		c = cola_extraer(q);

		if(c == '\0'){
			sem_unlink(SEM);
			sem_unlink(SEM_COLA_VACIA);
			sem_unlink(SEM_COLA_LLENA);

			munmap(q,sizeof(Queue*));
			shm_unlink(FILE);

			sem_close(sem);
			sem_close(sem_cola_vacia);
			sem_close(sem_cola_llena);
		}
		else{
			printf("%c ",c);
			fflush(stdout);
		}

		sem_post(sem);
		sem_post(sem_cola_llena);
	}

	return 0;
}