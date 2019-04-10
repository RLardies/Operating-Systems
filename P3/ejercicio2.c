#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define SEM "/mi_semaforo"
#define FILE "/mi_fichero"
#define NAME_MAX 1024

typedef struct {
	int previous_id; //!< Id of the previous client.
	int id; //!< Id of the current client.
	char name[NAME_MAX]; //!< Name of the client.
} ClientInfo;


ClientInfo *cinfo;
sem_t *sem;

void manejador_SIGUSR1(int sig) {
	printf("Id antiguo: %d\n", cinfo->previous_id);
	printf("Id actual: %d\n", cinfo->id);
	printf("Nombre: %s\n", cinfo->name);
	fflush(stdout);
	sem_post(sem);
}

int main(int argc, char *argv[]) {

	struct sigaction act;
	pid_t pid;
	sigset_t mask;	
	int fd, n = atoi(argv[1]), i;
	char name[NAME_MAX];

	act.sa_handler = manejador_SIGUSR1;
	act.sa_flags = 0;
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}

	if ((fd = shm_open(FILE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error en shm_open");
		exit(EXIT_FAILURE);
	}
	shm_unlink(FILE);

	if (ftruncate(fd, sizeof(ClientInfo)) < 0) {
		perror("Error en ftruncate");
		close(fd);
		exit(EXIT_FAILURE);
	}

	if ((cinfo = (ClientInfo *) mmap(NULL, sizeof(ClientInfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("Error en mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);
	cinfo->previous_id = -1;
	cinfo->id = 0;

	if ((sem = sem_open(SEM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
		perror("Error en sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink(SEM);

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	for (i = 0; i < n; i++) {
		if ((pid = fork()) < 0) {
			perror("Error en fork");
			munmap(cinfo, sizeof(ClientInfo));
			sem_close(sem);
			exit(EXIT_FAILURE);
		}
		if (pid == 0) {
			srand(time(NULL) + 12 * i);
			sleep(random() * 9 / RAND_MAX + 1);

			sem_wait(sem);
			cinfo->previous_id++;
			printf("Introduzca un nombre: ");
			scanf("%s", name);
			strcpy(cinfo->name, name);
			cinfo->id++;

			kill(getppid(), SIGUSR1);
			sem_close(sem);
			munmap(cinfo, sizeof(ClientInfo));
			exit(EXIT_SUCCESS);
		}
	}

	sigemptyset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	for (i = 0; i < n; i++) pause();

	while(wait(NULL) > 0);
	sem_close(sem);
	munmap(cinfo, sizeof(ClientInfo));
	return EXIT_SUCCESS;
}
