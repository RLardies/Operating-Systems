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
	
	int fd, i; 
	mqd_t queue;
	char *file;
	struct stat s;
	 
	if ((fd = open(argv[1], O_RDONLY, S_IRUSR) ) < 0 ) {
		perror("Error abriendo el archivo");
		exit(EXIT_FAILURE); 	
	}	
	
	if (fstat(fd, &s) < 0){
		perror("Error fstat");
		exit(EXIT_FAILURE);
	} 

	if ((file = (char *) mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED){
		perror("Error mmap");
	   	exit(EXIT_FAILURE); 	
	}
	
	if ((queue = mq_open(argv[2], O_WRONLY)) < 0) {
		perror("Error al abrir la cola");
	   	exit(EXIT_FAILURE); 	
	}

	for (i = 0; i < s.st_size; i += 2048) {
		if (mq_send(queue, file + i, 2048, 1) < 0) {
			perror("Error enviando");
			exit(EXIT_FAILURE);
		}
	}
	if (mq_send(queue, file + i - 2048, s.st_size - (i - 2048), 1) < 0) {
		perror("Error enviando");
		exit(EXIT_FAILURE);	
	}

	munmap(file, s.st_size);
	close(fd);	
	close(queue);		
	
	return EXIT_SUCCESS;
}
