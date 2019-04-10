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

#define QUEUE1 "/queue1"
#define QUEUE2 "/queue2"
#define FILE "prueba.txt"

int main() {
	
	struct mq_attr atr = {
		.mq_flags = 0,
		.mq_maxmsg = 10,
		.mq_msgsize = 2048,
		.mq_curmsgs = 0
	};
	pid_t pid;
	mqd_t q1, q2;

	if ((q1 = mq_open(QUEUE1, O_CREAT | O_EXCL | O_RDWR, S_IWUSR | S_IRUSR, &atr)) < 0) {
		perror("Error abriendo cola 1");
		exit(EXIT_FAILURE);
	}

	if ((q2 = mq_open(QUEUE2, O_CREAT | O_EXCL | O_RDWR, S_IWUSR | S_IRUSR, &atr)) < 0) {
		perror("Error abriendo cola 1");
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) < 0) {
		perror("Error en fork");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		execl("./A", "A", FILE, QUEUE1, (char *) NULL);
		perror("Error en exec");
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) < 0) {
		perror ("Error en el fork");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		execl("./B", "B", QUEUE1, QUEUE2, (char *) NULL);
		perror("Error en exec");
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) < 0) {
		perror ("Error en el fork");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		execl("./C", "C", QUEUE2, (char *) NULL);
		perror("Error en exec");
		exit(EXIT_FAILURE);
	}

	while(wait(NULL) > 0);

	mq_unlink(QUEUE1);
	mq_unlink(QUEUE2);
	close(q1);
	close(q2);	

	return EXIT_SUCCESS;
}
