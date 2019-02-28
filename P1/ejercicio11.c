/* uso del input y output estándar */
#include <stdio.h>
/* para la memoria dinámica */
#include <stdlib.h>
/* uso de strings */
#include <string.h>
/* incluye la función sleep */
#include <unistd.h>
/* creación y administración de hilos */
#include <pthread.h>

void *slowprintf (void *arg) {
   char *msg;
   int i;
   msg = (char *)arg;

   for ( i = 0 ; i < strlen(msg) ; i++ ) {
    printf(" %c", msg[i]);
    fflush(stdout);
    sleep (1) ;
   }
   pthread_exit(NULL);
}

int main(int argc , char *argv[]) {

   pthread_t h1;
   pthread_t h2;
   char *hola = "Hola ";
   char *mundo = "Mundo";

   /**
    * Creamos los hilos, cada uno llamando a la función slowprint con diferentes
    * palabras.
    */
   pthread_create(&h1, NULL , slowprintf , (void *)hola);
   pthread_create(&h2, NULL , slowprintf , (void *)mundo);

   /**
    * Esperamos a que finalicen los hilos creados anteriormente.
    */
   pthread_join(h1,NULL);
   pthread_join(h2,NULL);

   printf("El programa %s termino correctamente \n", argv[0]);
   exit(EXIT_SUCCESS);
}
