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


void manejador_SIGUSR1(int sig) {
}

int main(int argc, char *argv[]) {

	struct sigaction act;
	pid_t pid;
	sigset_t mask;	
	int fd, n = atoi(argv[1]), i;
	ClientInfo *cinfo;
	sem_t *sem;

	if (argc < 1) return EXIT_FAILURE;

	/*Establecemos el manejador de la señak SIGUSR1*/
	act.sa_handler = manejador_SIGUSR1;
	act.sa_flags = 0;
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}

	/*Abrimos la zona de memoria compartida*/
	if ((fd = shm_open(FILE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) < 0) {
		perror("Error en shm_open");
		exit(EXIT_FAILURE);
	}
	shm_unlink(FILE);

	/*Le asignamos el tamaño de ClienInfo*/
	if (ftruncate(fd, sizeof(ClientInfo)) < 0) {
		perror("Error en ftruncate");
		close(fd);
		exit(EXIT_FAILURE);
	}

	/*Mapeamos la región de memoria en la variable cinfo*/
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

	/*Establecemos la máscara de señales para evitar condición de carrera*/
	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	/*Creamos los procesos que vamos a utilizar*/
	for (i = 0; i < n; i++) {
		if ((pid = fork()) < 0) {
			perror("Error en fork");
			munmap(cinfo, sizeof(ClientInfo));
			sem_close(sem);
			exit(EXIT_FAILURE);
		}
		if (pid == 0) {
			/*Modificamos la semilla de los números aleatorios para que no salgan iguales*/
			srand(time(NULL) + 12 * i);
			/*Mandamos a dormir el proceso un tiempo aleatorio entre 1 y 10 segundos*/
			sleep(random() * 9 / RAND_MAX + 1);

			/*Solicitamos la información por pantalla protegiéndolo con un semáforo*/
			sem_wait(sem);
			cinfo->previous_id++;
			printf("Introduzca un nombre: ");
			scanf("%s", cinfo->name);
			cinfo->id++;

			/*Indicamos al padre que ya hemos terminado y liberamos recursos*/
			kill(getppid(), SIGUSR1);
			sem_close(sem);
			munmap(cinfo, sizeof(ClientInfo));
			exit(EXIT_SUCCESS);
		}
	}

	/*Quitamos SIGUSR1 de la máscara para poder recibirla durante el sigsuspend*/
	sigdelset(&mask, SIGUSR1);
	for (i = 0; i < n; i++) {
		/*Esperamos a que termine alguno de los hijos*/
		sigsuspend(&mask);
		/*Imprimimos la información por pantalla*/
		printf("Id antiguo: %d\n", cinfo->previous_id);
		printf("Id actual: %d\n", cinfo->id);
		printf("Nombre: %s\n", cinfo->name);
		fflush(stdout);
		/*Permitimos que siga el siguiente hijo*/
		sem_post(sem);
	}

	/*Esperamos a que terminen todos los hijos*/
	while(wait(NULL) > 0);
	sem_close(sem);
	munmap(cinfo, sizeof(ClientInfo));
	return EXIT_SUCCESS;
}
