#include <stdio.h>
#include <stdlib.h>

typedef struct _Queue{
	char *queue;
	int size;
	int first;
	int last;
} Queue;

int queue_init(Queue *qp, int size) {
	if ((qp->queue = (char *) malloc(size * sizeof(char))) == NULL)
		return -1;
	qp->size = size;
	qp->first = 0;
	qp->last = 0;

	return 0;
}

int cola_llena(Queue *qp) {
	if (qp->first == (qp->last + 1)%qp->size)
		return 0;
	return -1;
}

int cola_vacia(Queue *qp) {
	if (qp->first == qp->last) return 0;
	return -1;
}

int cola_insertar(Queue *qp, char elem) {
	if (cola_llena(qp) < 0) return -1;
	qp->queue[++(qp->last)%qp->size] = elem;
	return 0;
}

char cola_extraer(Queue *qp) {
	char aux;

	if (cola_vacia(qp) == 0) return -1;
	aux = qp->queue[qp->first];
	qp->first = (qp->first + 1)%(qp->size);

	return aux;
}
