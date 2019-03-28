#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

/* manejador: rutina de tratamiento de la señal SIGKILL. */
void manejador(int sig) {
	printf("He conseguido capturar SIGKILL\n");
	fflush(stdout);
}
int main(void) {

	struct sigaction act;

	act.sa_handler = manejador;
	sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
	/*Como la señal SIGKILL no es bloqueable la función sigaction devuelve error*/
	if (sigaction(SIGKILL, &act, NULL) < 0) {
		perror("Error en la funcion sigaction");
		exit(EXIT_FAILURE);
	}

	printf("Mandamos la señal SIGKILL: \n");
	kill(getpid(), SIGKILL);
}