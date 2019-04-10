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

	mqd_t queue;
	struct mq_attr atr;
	char buf[2048];
	unsigned int prior;
	
	if ((queue = mq_open(argv[1], O_RDONLY)) < 0) {
		perror("Error al abrir la cola");
		exit(EXIT_FAILURE); 	
	}
	
	mq_getattr(queue, &atr);
	while(atr.mq_curmsgs > 0) {
		memset(buf, 0, 2048);

		if (mq_receive(queue, buf, 2048, &prior) < 0) {
			perror("Error leyendo cola");
			exit(EXIT_FAILURE);
		}
		printf("%s\n", buf);
		mq_getattr(queue, &atr);
	}

	close(queue);

	return EXIT_SUCCESS;
}
