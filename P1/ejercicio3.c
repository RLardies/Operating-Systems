/* uso del input y output estándar */
#include <stdio.h>
/* para la memoria dinámica */
#include <stdlib.h>
/* incluye la función sleep y el fork*/
#include <unistd.h>
#include <sys/types.h>
/* incluye las función wait */
#include <sys/wait.h>

#define NUM_PROC 3

int main(void)
{
	pid_t pid;
	int i;
	for(i = 0; i < NUM_PROC; i++)
	{
		pid = fork();
		if(pid <  0)
		{
			printf("Error al emplear fork\n");
			exit(EXIT_FAILURE);
		}
		else if(pid ==  0)
		{
			sleep(i);
			printf("HIJO %d PADRE %d\n", getpid(), getppid());
			exit(EXIT_SUCCESS);
		}
		else if(pid >  0)
		{
			printf("PADRE %d\n", i);
			/**
			 * Poniendo aquí el wait nos aseguramos de que el padre espere a sus hijos
			 * antes de terminar para que no quede ningún proceso huérfano.
			 */
			wait(NULL);
		}
	}
	exit(EXIT_SUCCESS);
}

