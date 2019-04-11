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
#include <mqueue.h>

#define SIZEBUF 10

int main(int argc, char *argv[]) {
	
	int fd, i, multip = 0; 
	mqd_t queue;
	char *file;
	struct stat s;
	char aux = -1;
	/*Abrimos el archivo que vamos a leer*/ 
	if ((fd = open(argv[1], O_RDONLY, S_IRUSR) ) < 0 ) {
		perror("Error abriendo el archivo");
		exit(EXIT_FAILURE); 	
	}	
	
	/*Guardamos los datos del fichero para leer el tamaño*/	
	if (fstat(fd, &s) < 0){
		perror("Error fstat");
		exit(EXIT_FAILURE);
	} 
	/*Vemos si el tamaño del archivo es multiplo del del buffer para tenerlo en cuenta*/
	if (s.st_size % SIZEBUF == 0) multip = 1;

	/*Mapeamos la información del fichero de forma que apunte a un puntero a char*/
	if ((file = (char *) mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED){
		perror("Error mmap");
	   	exit(EXIT_FAILURE); 	
	}
	
	if ((queue = mq_open(argv[2], O_WRONLY)) < 0) {
		perror("Error al abrir la cola");
	   	exit(EXIT_FAILURE); 	
	}

	/*Vamos leyendo el archivo con el buffer y metiéndolo en la cola*/
	for (i = 0; i < s.st_size; i += SIZEBUF) {
		if (i + SIZEBUF > s.st_size) {
			/*Cuando sea el último envío, enviamos solo lo que falta*/
			if (mq_send(queue, file + i, s.st_size - i, 1) < 0) {
				perror("Error enviando A");
				exit(EXIT_FAILURE);	
			}
		}		
		else if (mq_send(queue, file + i, SIZEBUF, 1) < 0) {
			perror("Error enviando A");
			exit(EXIT_FAILURE);
		}
	}
	/*Si el tamaño del fichero era múltiplo del del buffer mandamos un caracter -1 extra*/
	if (multip == 1) {
		if (mq_send(queue, &aux, 1, 1) < 0) {
			perror("Error enviando A");
			exit(EXIT_FAILURE);	
		}
	}

	munmap(file, s.st_size);
	close(fd);	
	close(queue);		
	
	return EXIT_SUCCESS;
}
