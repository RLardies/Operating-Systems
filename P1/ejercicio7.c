/**
 * Archivo: ejercicio7.c
 * 
 * Autores: Carlos Gómez-Lobo Hernaiz -- carlos.gomez-lobo@estudiante.uam.es
 *          Rodrigo Lardiés Guillén   -- rodrigo.lardies@estudiante.uam.es
 *
 *  Grupo: 2202
 *
 *  Fecha: 01/02/2019
 *
 *  Este ejercicio es un ejemplo de ejecución de los comandos exec, mostrando los
 *  ficheros del directorio actual, pidiendo algunos en concreto y mostrando su contenido.
 */

/* wait and return process info */
#include <sys/types.h>
/* funciones que esperan que terminen procesos */
#include <sys/wait.h>
/* standard input/output */
#include <stdio.h>
/* malloc, free */
#include <stdlib.h>
/* library for exec */
#include <unistd.h>
/*for comparing strings*/
#include<string.h>

/*
* Pide al usuario una serie de ficheros separados por comas, los almacena en
* un vector de strings, crea un nuevo proceso y ejecuta el comando cat con el
* vector de strings como vector de argumentos
*/
void  processCat () {

    /* Variables que usa el metodo getline para leer la entrada del usuario */
    char *fileName = NULL;
    size_t fileLen = 0;
    ssize_t fileRead;

    /* pide al usuario una linea de texto con todos los ficheros separados por comas */
    printf("Introduzca los ficheros que quiere mostrar separados por ',' \n");
    while((fileRead = getline(&fileName, &fileLen, stdin)) < 1)
    {
   	 printf("Por favor inserte al menos un fichero \n");
    }

    /* Cuenta el número de ficheros */
    size_t fileCount = 0;
    for(ssize_t i = 0; i < fileRead; i++)
    {
   	 if(fileName[i] == ',' || fileName[i] == '\n')
   	 {
   		 fileCount++;
   	 }
    }

    size_t nArgs = fileCount + 2;
    /* Reserva espacio para argumentos */
    char ** args = malloc(nArgs * sizeof(*args));
    if(args == NULL)
    {
   	 exit(EXIT_FAILURE);
    }

    args[0] = "cat";

    char * filePtr = fileName;
    size_t argIndex = 1;
    for(ssize_t i = 0; i < fileRead; i++)
    {
   	 if(fileName[i] == ',' || fileName[i] == '\n')
   	 {
   		 fileName[i] = '\0';
   		 args[argIndex] = filePtr;
   		 argIndex++;
   		 filePtr = &fileName[i + 1];
   	 }
    }

    args[nArgs - 1] = NULL;

    if (nArgs >  1) {
   	 
     pid_t hijo;

     /**
      * Cremos el proceso que va a realizar el execvp.
      */
     if ((hijo = fork()) < 0) exit(EXIT_FAILURE);

     else if (hijo == 0){

       execvp("cat", args);
       /**
        * La única razón por la que esta parte del código puede ejecutarse es porque
        * ha habido algún error en la realización del exec, por tanto termina el 
        * proceso con error.
        */
       printf("Error realizando el exec.\n");
       exit (EXIT_FAILURE);
     }
    }
    /* Liberamos la memoria dinamica reservada por el proceso */
    free (args);
    /* liberamos la memoria reservada por getline */
    free (fileName);
}

void  showAllFiles () {
    
    pid_t hijo;

    /**
      * Cremos el proceso que va a realizar el execlp.
      */
    if ((hijo = fork()) < 0) exit(EXIT_FAILURE);

    else if (hijo == 0){

      execlp("ls", "ls", "-l", NULL);
      /**
        * La única razón por la que esta parte del código puede ejecutarse es porque
        * ha habido algún error en la realización del exec, por tanto termina el 
        * proceso con error.
        */
      printf("Error realizando el exec.\n");
      exit (EXIT_FAILURE);
    }
}

int  main(void) {
    showAllFiles();
    processCat();
    exit (EXIT_SUCCESS);
}
