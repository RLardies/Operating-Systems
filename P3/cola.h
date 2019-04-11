#ifndef QUEUE_H
#define QUEUE_H
#define MAX_COLA 10

typedef struct _Queue{
	char queue[MAX_COLA];
	int tail;
	int head;
} Queue;
typedef struct _Queue Queue;

int cola_init(Queue *qp);
int cola_llena(Queue *qp);
int cola_vacia(Queue *qp);
int cola_insertar(Queue *qp, char elem);
int cola_extraer(Queue *qp, char *ret);

#endif
