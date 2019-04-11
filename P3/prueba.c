#include "cola.h"
#include <stdio.h>
#include <stdlib.h>

void print_cola(Queue *q) {
	printf("%d %d\n", q->tail, q->head);
	for (int i = q->tail; i < q->head; i++)
		printf("%c", q->queue[i]);
	printf("Impreso\n");
}

int main() {

	Queue *q;
	char c;

	q = (Queue *) malloc(sizeof(Queue));
	cola_init(q);

	for (char i = 0; i < 10; i++) {
		cola_insertar(q, 65 + i);
	}
	printf("Imprimimos cola\n");
	print_cola(q);
	if (cola_insertar(q, 78) == 0)
		printf("Error al insertar le 10\n");
	for (int i = 0; i < 10; i++) {
		cola_extraer(q, &c);	
		printf("%c", c);
	}
	if (cola_extraer(q, &c) == 0)
		printf("Error al extraer el 10\n");

	return 0;
}
