#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5

int main(void){

	pid_t pid;
	int counter;
	sigset_t mask1, mask2;

	pid = fork();
	if(pid < 0){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0) {

		sigemptyset(&mask1);
		sigaddset(&mask1, SIGUSR1);
		sigaddset(&mask1, SIGUSR2);
		sigaddset(&mask1, SIGALRM);

		sigemptyset(&mask2);
		sigaddset(&mask2, SIGUSR1);
		sigaddset(&mask2, SIGALRM);

		alarm(40);

		while(1) {
			sigprocmask(SIG_SETMASK, &mask1, NULL);

			for(counter = 0; counter < N_ITER; counter++) {
				printf("%d\n", counter);
				sleep(1);
			}

			sigprocmask(SIG_UNBLOCK, &mask2, &mask1);
			sleep(3);
		}
	}
	while(wait(NULL) > 0);
}