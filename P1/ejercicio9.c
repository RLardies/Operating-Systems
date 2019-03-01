/**
 * Archivo: ejercicio9.c
 * 
 * Autores: Carlos Gómez-Lobo Hernaiz -- carlos.gomez-lobo@estudiante.uam.es
 *		 	Rodrigo Lardiés Guillén   -- rodrigo.lardies@estudiante.uam.es
 *
 *	Grupo: 2202
 *
 *	Fecha: 01/02/2019
 *
 *	Este programa es un ejemplo de comunicación entre procesos a través de pipes,
 *	haciendo el proceso padre de mediador entre sus procesos hijos.
 */
 
/* uso del input y output estándar */
#include <stdio.h>
/* para la memoria dinámica */
#include <stdlib.h>
/* incluye la función sleep y el fork*/
#include <unistd.h>
#include <sys/types.h>
/* incluye las función wait */
#include <sys/wait.h>
/* incluye la función time */
#include <time.h>

int main(int argc, char *argv[]) {
  

	pid_t pid1,pid2;
	int fd1[2],fd2[2];
 	int pipe_status,buffer;
 	int r;

 	srand(time(NULL));
	
 	/**
 	 * Creamos el pipe que va a comunicarse con el proceso que genera y envía el número.
 	 */
	pipe_status = pipe(fd1);
	if(pipe_status == -1){
		perror("Error creando la tuberia\n");
	}
	/**
 	 * Creamos el pipe que va a comunicarse con el proceso que reciba el número.
 	 */
	pipe_status = pipe(fd2);
	if(pipe_status == -1){
		perror("Error creando la tuberia\n");
	}
	/**
	 * Creamos el proceso que va a generar y enviar el número.
	 */
	if( (pid1 = fork()) == -1){
		perror("Error creando el proceso hijo\n");
		exit(EXIT_FAILURE);
	}

	if(pid1 == 0){
		/**
		 * El prceso que va a generar el número y enviarlo cierra el pipe 2 y el canal
		 * de lectura del pipe 1 ya que no los va a utilizar.
		 */
		close(fd2[0]);
		close(fd2[1]);
		close(fd1[0]);
		/**
		 * Se genera el número aleatorio.
		 */
		r = rand();
		printf("Este proceso ha generado el numero aleatorio: %d\n", r);

		/**
		 * @brief      Se escribe el número en el canal de escritura del pipe 1.
		 *
		 * @param      destino    Canal de escritura del pipe 1.
		 * @param      msg        Número aleatorio.
		 * @param[in]  size  	  Tamaño del número.
		 */
		write(fd1[1],&r,sizeof(r));
		exit(EXIT_SUCCESS);
	}
	/**
	 * Creamos el proceso que va a recibir el mensage
	 * (solo llega a este punto el padre del primero).
	 */
	if( (pid2 = fork()) == -1){
		perror("Error creando el segundo proceso hijo\n");
		exit(EXIT_FAILURE);
	}
	
	if(pid2 == 0){
		/**
		 * El hijo cierra los canales de los pipes que no va a utilizar, dejando
		 * solo el canal de lectura del pipe 2.
		 */
		close(fd2[1]);
		close(fd1[1]);
		close(fd1[0]);
		/**
		 * @brief      Leemos el número aleatorio del pipe.
		 *
		 * @param      origen  	  Canal del pipe del que leemos el número.
		 * @param      buffer     Variable en la que vamos a guardar el número leído.
		 * @param[in]  size       Tamaño del número recibido.
		 */
		read(fd2[0],&buffer,sizeof(buffer));

		printf("El numero leido en el hijo 2 es: %d\n",buffer);
		exit(EXIT_SUCCESS);
	}
	/**
	 * El proceso padre cierra los canales de los pipes que no va a utilizar y hace 
	 * de intermediario del mensaje, recibiendo el mensaje del primer pipe y escribiéndolo
	 * en el segundo.
	 */
	close(fd1[1]);
	close(fd2[0]);

	read(fd1[0],&buffer,sizeof(buffer));
	write(fd2[1],&buffer,sizeof(buffer));

	/**
	 * Esperamos a que finalicen los procesos hijos y cerramos los canales de los pipes
	 * que quedan.
	 */
	wait(NULL);
	wait(NULL);
	close(fd2[1]);
	close(fd1[0]);
 
  exit(EXIT_SUCCESS);
}
