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

#define SIZEBUF 2048

int main(int argc, char *argv[]) {

	mqd_t queue;
	char buf[SIZEBUF];
	unsigned int prior;
	int flag = 0;
	ssize_t n;
	
	/*Abrimos la cola de la que vamos a recibir la informacion*/
	if ((queue = mq_open(argv[1], O_RDONLY)) < 0) {
		perror("Error al abrir la cola");
		exit(EXIT_FAILURE); 	
	}
	
	/*Iniciamos el bucle que va a leer e imprimir la informacion*/
	while(flag == 0) {
		if ((n = mq_receive(queue, buf, SIZEBUF, &prior)) < 0) {
			perror("Error leyendo cola C");
			exit(EXIT_FAILURE);
		}
		/*Cuando le llegue la última cadena pone la flag que controla el bucle a 1*/
		if (n < SIZEBUF) {
			flag = 1;
			/*Si la última cadena solo es un -1, significa que el tamaño del archivo era
			 * multiplo del tamaño del bufer, y este no lo tiene que imprimir*/
			if (n == 1 && buf[0] == -1)
				break;
		}
		/*Imprimimos la información por pantalla*/
		write(1, buf, n);
	}

	close(queue);

	return EXIT_SUCCESS;
}
