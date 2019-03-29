#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define PROC_NUM 4

int main (){

	int i, pid;

	for (i = 0; i < PROC_NUM; i++){

		if ((pid = fork()) < 0){

			perror("Error creando el proceso.\n");
			exit (EXIT_FAILURE);
		}
		else if (pid == 0){

			printf("Soy el proceso hijo %d\n", getpid());
			sleep(30);

			printf("Soy el proceso hijo %d y ya me toca terminar.\n", getpid());
			exit(EXIT_SUCCESS);
		}
		else
			sleep(5);
			kill(pid, SIGTERM);
	}

	return EXIT_SUCCESS;
}