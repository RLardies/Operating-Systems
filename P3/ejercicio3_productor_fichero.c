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
#define FILE "mi_fichero"
#define MAX_COLA 10

int main(int argc, char *argv[]) {

	int fd, flag = 0;
	char c;
	Queue *q;
	sem_t *sem;
	sem_t *sem_cola_vacia;
	sem_t *sem_cola_llena;

	/*Creamos los semáforos que vamos a utilizar*/
	if ((sem = sem_open(SEM, O_RDWR | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("Error en sem_open");
		exit(EXIT_FAILURE);
	}

	if ((sem_cola_llena = sem_open(SEM_COLA_LLENA, O_RDWR | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, MAX_COLA)) == SEM_FAILED) {
		perror("Error en sem_open");
		exit(EXIT_FAILURE);
	}

	if ((sem_cola_vacia = sem_open(SEM_COLA_VACIA, O_RDWR | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
		perror("Error en sem_open");
		exit(EXIT_FAILURE);
	}

	/*Creamos el FICHERO en el que vamos a almacenar la cola*/
	if ((fd = open(FILE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error en open");
		exit(EXIT_FAILURE);
	}

	if (ftruncate(fd, sizeof(Queue)) < 0) {
		perror("Error en ftruncate");
		close(fd);
		exit(EXIT_FAILURE);
	}

	if ((q = (Queue *) mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

	/*Inicializamos la cola*/
	if(cola_init(q) < 0){
		perror("cola_init");
		exit(EXIT_FAILURE);
	}
	
	/*Inicializamos el bucle que solicitará los caracteres y los introducirá en la cola*/
	printf("Introduce un caracter: ");
	while(flag == 0){
		c = (char) getchar();
		if (c != '\n') {
			sem_wait(sem_cola_llena);
			sem_wait(sem);
			/*Si se introduce el EOF se inserta en la cola '\0' y se finaliza el bucle*/
			if (c == EOF) {
				cola_insertar(q, '\0');
				flag = 1;
				printf("\n");
			}
			/*Si no es EOF se introduce el caracter en la cola*/
			else {
				cola_insertar(q,c);
				printf("Introduce un caracter: ");
			}
			sem_post(sem_cola_vacia);
			sem_post(sem);
		}
	}

	/*Liberamos los recursos y finalizamos el programa*/
	unlink(FILE);
	sem_unlink(SEM);
	sem_unlink(SEM_COLA_VACIA);
	sem_unlink(SEM_COLA_LLENA);

	munmap(q, sizeof(Queue));

	sem_close(sem);
	sem_close(sem_cola_vacia);
	sem_close(sem_cola_llena);

	return 0;
}
