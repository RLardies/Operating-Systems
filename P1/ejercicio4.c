#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROG 3

int main(void)
{
	pid_t pid = 0;
	int i, wstatus;
	
	for (i = 0; i < NUM_PROG; i++)
		if (pid == 0) pid = fork();

	waitpid(pid, &wstatus, 0);
	exit(EXIT_SUCCESS);
}