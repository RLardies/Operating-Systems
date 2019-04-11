#include <stdio.h>
#include <stdlib.h>
#include "cola.h"


int cola_init(Queue *qp) {
	if (qp == NULL) return -1;
	qp->tail = -1;
	qp->head = 0;
	return 0;
}

int cola_llena(Queue *qp) {
	if (qp->tail == qp->head)
		return 0;
	return -1;
}

int cola_vacia(Queue *qp) {
	if (qp->tail == -1) return 0;
	return -1;
}

int cola_insertar(Queue *qp, char elem) {
	if (cola_llena(qp) == 0) return -1;
	if (cola_vacia(qp) == 0) {
		qp->tail = 0;
		qp->head = 0;
	}
	printf("Insertamos\n");
	qp->queue[qp->head] = elem;
	qp->head = (qp->head + 1) % MAX_COLA;
	return 0;
}

int cola_extraer(Queue *qp, char *ret) {
	if (cola_vacia(qp) == 0) return -1;
	*ret = qp->queue[qp->tail];
	if ((qp->tail + 1)%MAX_COLA == qp->head) qp->tail = -1;
	else qp->tail = (qp->tail + 1)%(MAX_COLA);

	return 0;
}
