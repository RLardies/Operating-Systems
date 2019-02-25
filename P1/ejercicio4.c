#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROC 3

int main(void)
{
	pid_t pid;
	int i, wstatus;

	for(i = 0; i < NUM_PROC; i++) {
		pid = fork();
		if(pid <  0)
		{
			printf("Error al emplear fork\n");
			exit(EXIT_FAILURE);
		}
		else if(pid >  0)
		{
			printf("HIJO %d PADRE %d\n", getpid(), getppid());
			waitpid(pid, &wstatus, 0);
			exit(EXIT_SUCCESS);
		}
		else if(pid ==  0)
		{
			printf("PROC %d\n", i);
		}
	}
	exit(EXIT_SUCCESS);
}