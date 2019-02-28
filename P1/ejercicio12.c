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
/* se usa para la función pow */
#include <math.h>

#define N 10

/**
 * Esta es la estructura que se pasará a los hilos como argumento, que 
 * contiene las variables necesarias para cumplir su función.
 */
typedef struct {
  int x;
  int output;
}Args;

/**
 * @brief      Función que calcula la potencia de 2 pasado como argumento.
 *
 * @param      arg   La variable de la estructura Args que contiene el exponente y la 
 *                   variable en la que se deberá guardar el resultado.
 */
void power_2 (void *arg){

  Args *args = arg;

  args->output = pow(2, args->x);
}

int main(int argc , char *argv[]) {

  pthread_t h[N];
  Args args[N];
  int i;

  for(i=0;i<N;i++){
    args[i].x = i;
    pthread_create(h + i, NULL, (void *) power_2, args + i);
  }

  for (i = 0; i < N; i++){

    pthread_join(h[i], NULL);
    printf ("El valor del hilo %d es: %d\n", i, args[i].output);
  }

  exit(EXIT_SUCCESS);
}
