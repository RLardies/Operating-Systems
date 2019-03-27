#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define SEM "/mi_sefamoro"
#define N_PROC 5
#define FILENAME "carrera"

int check_file(int fd) {

	int buf, acum[N_PROC] = { 0 }, ret = -1, i;

	lseek(fd, 0, SEEK_SET);
	while (read(fd, &buf, sizeof(buf)) > 0)
		if (++acum[buf] == 20 && ret == -1) ret = buf;

	for (i = 0; i < N_PROC; i++)
		printf("Puntuacion del proceso %d: %d\n", i, acum[i]);
	printf("\n\n");

	if (ret == -1)
		close(open(FILENAME, O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR));

	return ret;
}

int main() {

	pid_t pid;
	sem_t *sem;
	int i, file, res;
	sigset_t mask;

	if ((sem = sem_open(SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
		perror("Error creando semaforo");
		exit(EXIT_FAILURE);
	}
	/*Hacemos unlink del semaforo para que cuando terminen todos los procesos se borre*/
	sem_unlink(SEM);

	for (i = 0; i < N_PROC; i++) {
		if ((pid = fork()) < 0) {
			perror("Error creando proceso");
			exit(EXIT_FAILURE);
		}
		if (pid == 0) {
			/*Modificamos la semilla para que no tengan todos la misma y que sea distinta 
			 cada vez que se ejecute*/
			srand((time(NULL) * i + 200));

			while(1) {
				sem_wait(sem);

				if ((file = open(FILENAME, O_CREAT | O_APPEND | O_WRONLY , S_IRUSR | S_IWUSR)) < 0) {
					perror("Error abriendo el archivo");
					sem_post(sem);
					sem_close(sem);
					exit(EXIT_FAILURE);
				}
				if (write(file, &i, sizeof(i)) != sizeof(i)) {
					perror("Error escribiendo en el archivo");
					sem_post(sem);
					sem_close(sem);
					close(file);
					exit(EXIT_FAILURE);
				}
				if (close(file) < 0) {
					perror("Error abriendo el archivo");
					sem_post(sem);
					sem_close(sem);
					exit(EXIT_FAILURE);
				}

				sem_post(sem);
				/*Hacemos un usleep de un numero aleatorio entre 0 y 100000 microsegundos (0.1 segundos)*/
				usleep(random() * 100000.0 / RAND_MAX);
			}
		}
	}

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	if ((file = open(FILENAME, O_CREAT | O_RDONLY | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error abriendo el archivo");
		sem_close(sem);
		exit(EXIT_FAILURE);
	}

	while (1) {
		sem_post(sem);
		sleep(1);
		sem_wait(sem);

		if ((res = check_file(file)) >= 0) {
			printf("La carrera a terminado y el ganador ha sido el proceso %d.\n", res);
			kill(0, SIGTERM);

			while(wait(NULL) > 0);
			sem_close(sem);
			close(file);
			exit(EXIT_SUCCESS);
		}
	}
}