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


int main(int argc, char *argv[]) {

	mqd_t rqueue, wqueue;
	struct mq_attr atr;
	char buf[2048];
	int i; 
	unsigned int prior;

	if ((rqueue = mq_open(argv[1], O_RDONLY)) < 0) {
		perror("Error al abrir la cola");
	   	exit(EXIT_FAILURE); 	
	}

	if ((wqueue = mq_open(argv[2], O_WRONLY)) < 0) {
		perror("Error al abrir la cola");
	   	exit(EXIT_FAILURE); 	
	}

	mq_getattr(rqueue, &atr);
	while(atr.mq_curmsgs > 0) {
		mq_getattr(rqueue, &atr);
		memset(buf, 0, 2048);

		if (mq_receive(rqueue, buf, 2048, &prior) < 0) {
			perror("Error leyendo cola");
			exit(EXIT_FAILURE);
		}
		for (i = 0; buf[i] != 0 && i < 2048; i++) {
			if (buf[i] == 'z')
				buf[i] = 'a';
			else
				buf[i]++;
		}
		if (mq_send(wqueue, buf, 2048, 1) < 0) {
			perror("Error enviando a cola");
			exit(EXIT_FAILURE);
		}
		memset(buf, 0, 2048);
	}

	close(rqueue);
	close(wqueue);

	return EXIT_SUCCESS;
}
