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
	int i, wstatus;

	for(i = 0; i < NUM_PROC; i++) {
		pid = fork();
		if(pid <  0)
		{
			printf("Error al emplear fork\n");
			exit(EXIT_FAILURE);
		}
		/**
		 * Ahora invertimos las condiciones que había en los apartados 3 y 4 para que
		 * sean ahora los hijos los que crean sucesivamente los procesos.
		 */
		else if(pid >  0)
		{
			printf("HIJO %d PADRE %d\n", getpid(), getppid());
			/**
			 * Al poner aquí un waitpid aseguramos que cada proceso no termina hasta que
			 * no lo hace su hijo y no cualquier otro proceso.
			 */
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