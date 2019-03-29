#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_READ 5
#define SECS 5
#define SEM "/sem_lesctura"
#define SEM2 "/sem_escritura"
#define SEM3 "/sem_lectores"

void manejador_SIGINT(int sig) {
	int i;
	kill(0,SIGTERM);
	for(i = 0; i < N_READ; i++){
		wait(NULL);
	}
	exit(EXIT_SUCCESS);
}

void lectura(pid_t pid){
	printf("R-INI: %d\n",pid);
	sleep(1);
	printf("R-FIN: %d\n",pid);
}
void escritura(pid_t pid){
	printf("W-INI: %d\n",pid);
	sleep(1);
	printf("W-FIN: %d\n",pid);
}
int main(void) {


	sem_t *sem_lectura = NULL;
	sem_t *sem_escritura = NULL;
	sem_t *sem_lectores = NULL;
	pid_t pid;
	int i, sem_val;
	struct sigaction act;

	if ((sem_lectura = sem_open(SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("sem_open1");
		exit(EXIT_FAILURE);
	}
	sem_unlink(SEM);

	if ((sem_escritura = sem_open(SEM2, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("sem_open2");
		exit(EXIT_FAILURE);
	}
	sem_unlink(SEM2);

	if ((sem_lectores = sem_open(SEM3, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("sem_open3");
		exit(EXIT_FAILURE);
	}
	sem_unlink(SEM3);

	

	for(i = 0; i < N_READ; i++){
		pid = fork();

		if(pid < 0){
			perror("fork");
			exit(EXIT_FAILURE);			
		}
		else if(pid == 0){
			printf("Hijo numero %d\n",i+1);

			while(1){	
				sem_wait(sem_lectura);
				sem_post(sem_lectores);

				if(sem_getvalue(sem_lectores,&sem_val) == -1){
					perror("sem_getvalue");
					exit(EXIT_FAILURE);
				}

				if(sem_val == 1) sem_wait(sem_escritura);

				sem_post(sem_lectura);
				lectura(getpid());
				sem_wait(sem_lectura);
				sem_wait(sem_lectores);

				if(sem_getvalue(sem_lectores,&sem_val) == -1){
					perror("sem_getvalue");
					exit(EXIT_FAILURE);
				}

				if(sem_val == 0){
					sem_post(sem_escritura);
				}
				sem_post(sem_lectura);

				sleep(SECS);
			}
		}
	}

	sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;

	act.sa_handler = manejador_SIGINT;
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	act.sa_handler = SIG_IGN;
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	while(1){
		sem_wait(sem_escritura);
		escritura(getpid());
		sem_post(sem_escritura);
		sleep(SECS);
	}

	return 0;
}