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
sem_t *sem_cola_vacia;
sem_t *sem_cola_llena;


int main(int argc, char *argv[]) {

	int fd;
	char c;

	if ((sem = sem_open(SEM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("Error en sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink(SEM);

	if ((sem_cola_llena = sem_open(SEM_COLA_LLENA, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
		perror("Error en sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink(SEM_COLA_LLENA);

	if ((sem_cola_vacia = sem_open(SEM_COLA_VACIA, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, MAX_COLA)) == SEM_FAILED) {
		perror("Error en sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink(SEM_COLA_VACIA);


	if ((fd = shm_open(FILE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error en shm_open");
		exit(EXIT_FAILURE);
	}
	shm_unlink(FILE);

	if (ftruncate(fd, sizeof(q)) < 0) {
		perror("Error en ftruncate");
		close(fd);
		exit(EXIT_FAILURE);
	}

	if ((q = (Queue *) mmap(NULL, sizeof(q), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}

	if(queue_init(q,MAX_COLA)<0){
		perror("queue_init");
		close(fd);
		exit(EXIT_FAILURE);
	}
	
	while(fscanf(stdin,"%c",&c) != EOF){
		sem_wait(sem);
		sem_wait(sem_cola_vacia);
		cola_insertar(q,c);
		sem_post(sem);
		sem_post(sem_cola_llena);
	}

	sem_wait(sem);
	sem_wait(sem_cola_vacia);
	cola_insertar(q,'\0');
	sem_post(sem);
	sem_post(sem_cola_llena);

	sem_unlink(SEM);
	sem_unlink(SEM_COLA_VACIA);
	sem_unlink(SEM_COLA_LLENA);

	munmap(q,sizeof(Queue*));
	shm_unlink(FILE);

	sem_close(sem);
	sem_close(sem_cola_vacia);
	sem_close(sem_cola_llena);

	return 0;
}