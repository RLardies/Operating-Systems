#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <semaphore.h>

#include <simulador.h>
#include <gamescreen.h>
#include <mapa.h>

tipo_mapa *mapa;
sem_t *sem_inicio;

void manejador_SIGINT(int signal){
	screen_end();
	sem_close(sem_inicio);
	munmap(mapa,sizeof(mapa));
	exit(EXIT_FAILURE);
}

void mapa_print(tipo_mapa *mapa)
{
	int i,j;

	for(j=0;j<MAPA_MAXY;j++) {
		for(i=0;i<MAPA_MAXX;i++) {
			tipo_casilla cas=mapa_get_casilla(mapa,j, i);
			//printf("%c",cas.simbolo);
			screen_addch(j, i, cas.simbolo);
		}
		//printf("\n");
	}
	screen_refresh();
}


int main() {

	int shmfd;
	tipo_mapa * mapa;


	struct sigaction act;


    act.sa_handler = manejador_SIGINT;
	sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        fprintf(stderr, "Error en sigaction\n");
        return EXIT_FAILURE;
    }	

    if ((sem_inicio = sem_open(SEM_INICIO, O_CREAT , S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
    	fprintf(stderr, "Error creando el semáforo\n");
    	return EXIT_FAILURE;
    }

    if ((shmfd = shm_open(SHM_MAP_NAME, O_RDONLY, 0)) < 0) {
		fprintf(stderr, "Error abriendo la memoria compartida\n");
		sem_close(sem_inicio);
		return EXIT_FAILURE;
	}

	mapa = (tipo_mapa*)mmap(NULL,sizeof(tipo_mapa),PROT_READ,MAP_SHARED,shmfd,0);

	if(mapa == NULL){
		fprintf(stderr,"Error al acceder a la memoria compartida");
		sem_close(sem_inicio);
		close(shmfd);
		return EXIT_FAILURE;
	}


	screen_init();

	while(mapa->finalizado == false){ //Hay q poner algo para que acabe cuando termine la partida
		mapa_print(mapa);
		usleep(SCREEN_REFRESH);
	}

	screen_end();

	close(shmfd);
	sem_close(sem_inicio);
	munmap(mapa,sizeof(mapa));

	exit(EXIT_SUCCESS);
}
