#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <mqueue.h>

#define SIZEBUF 2048

int main(int argc, char *argv[]) {

	mqd_t rqueue, wqueue;
	char buf[SIZEBUF];
	int i, flag = 0;
	ssize_t n;
	unsigned int prior;

	/*Abrimos las colas que vamos a utilizar para que se comuniquen los procesos*/
	if ((rqueue = mq_open(argv[1], O_RDONLY)) < 0) {
		perror("Error al abrir la cola");
	   	exit(EXIT_FAILURE); 	
	}

	if ((wqueue = mq_open(argv[2], O_WRONLY)) < 0) {
		perror("Error al abrir la cola");
	   	exit(EXIT_FAILURE); 	
	}

	/*Vamos leyendo de una cola, transcribiéndolo y metiéndolo en la otra*/
	while (flag == 0) {
		if ((n = mq_receive(rqueue, buf, SIZEBUF, &prior)) < 0) {
			perror("Error leyendo cola B");
			exit(EXIT_FAILURE);
		}
		/*Solo si está entre la a y la z se suma 1*/
		for (i = 0; buf[i] != 0 && i < SIZEBUF; i++) {
			if (buf[i] == 'z')
				buf[i] = 'a';
			if (buf[i] >= 'a' && buf[i] < 'z')
				buf[i]++;
		}
		/*Si es el último envío se enviará solo lo necesario y se pondrá a 1 la flag que
		 * indica el fin del bucle*/
		if (n < SIZEBUF) {
			if (mq_send(wqueue, buf, n, 1) < 0) {
				perror("Error enviando a cola B");
				exit(EXIT_FAILURE);
			}
			flag = 1;
		}
		else if (mq_send(wqueue, buf, SIZEBUF, 1) < 0) {
			perror("Error enviando a cola B");
			exit(EXIT_FAILURE);
		}
	}

	close(rqueue);
	close(wqueue);

	return EXIT_SUCCESS;
}
