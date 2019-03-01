/**
 * Archivo: ejercicio5.c
 * 
 * Autores: Carlos Gómez-Lobo Hernaiz -- carlos.gomez-lobo@estudiante.uam.es
 *			Rodrigo Lardiés Guillén   -- rodrigo.lardies@estudiante.uam.es
 *
 *	Grupo: 2202
 *
 *	Fecha: 01/02/2019
 *
 *	En este ejercicio comprobamos que dos procesos distintos no comparten memoria 
 *	reservando memoria en uno y modificándola en otro, y viendo que solo es modificada
 *	en ese preoceso y no en el padre. Además vemos que, al ser memorias individuales, 
 *	hay que liberarlas por separado.
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
/* uso de strings */
#include <string.h>

int  main ( void )
{
	pid_t pid;
	char * sentence = (char *)malloc(5 * sizeof (char));

	pid = fork();
	if (pid <  0  )
	{
		printf("Error al emplear fork\n");
		exit (EXIT_FAILURE);
	}
	else  if(pid ==  0)
	{
		/**
		 * Escribe en la memoria individual del hijo, por lo que la variable sentence del
		 * padre no se ve alterada
		 */
		strcpy(sentence, "hola");
		/**
		 * Liberamos la memoria reservada para el proceso hijo (distinta de la 
		 * reservada para el prceso padre, aunque tenga la misma dirección relativa).
		 */
		free (sentence);
		exit(EXIT_SUCCESS);
	}
	else
	{
		wait(NULL);
		/**
		 * El printf no imprime nada dado que en este proceso no se ha escrito nada en el espacio
		 * de memoria reservado.
		 */
		printf("Padre: %s\n", sentence);
		/**
		 * Liberamos la memoria reservada en el proceso padre con el malloc hecho al inicio
		 * del programa.
		 */
		free (sentence);
		exit(EXIT_SUCCESS);
	}
}
