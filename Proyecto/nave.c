#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "mapa.h"


int main (int argc, char *argv[]) {

	int pipejefe = atoi(argv[1]);
	mqd_t cola;

	if ((cola = mq_open(QUEUE_NAME, O_WRONLY)) < 0) {
		perror("Error abriendo cola nave");
		kill(0, SIGTERM);
	}	

	return EXIT_SUCCESS;
}
