#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5

void manejador_SIGTERM(int sig) {

	printf("Soy %lld y he recibido la señal SIGTERM.\n", (long long) getpid());
	exit(EXIT_SUCCESS);
}

void manejador_SIGALRM(int sig) {
}

int main(void){

	pid_t pid;
	int counter;
	struct sigaction act;
	sigset_t mask;

	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	pid = fork();
	if(pid < 0){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0) {

		act.sa_handler = manejador_SIGTERM;
		if (sigaction(SIGTERM, &act, NULL) < 0) {
			perror("Error en sigaction");
			exit(EXIT_FAILURE);
		}
		
		while(1) {

			sigaddset(&mask, SIGTERM);
			sigprocmask(SIG_SETMASK, &mask, NULL);

			for(counter = 0; counter < N_ITER; counter++) {
				printf("%d\n", counter);
				sleep(1);
			}

			sigdelset(&mask, SIGTERM);
			sigprocmask(SIG_SETMASK, &mask, NULL);
			sleep(3);
		}
	}

	act.sa_handler = manejador_SIGALRM;
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}
	sigdelset(&mask, SIGALRM);

	alarm(40);
	sigsuspend(&mask);

	kill(pid, SIGTERM);

	while(wait(NULL) > 0);
}