#ifndef QUEUE_H
#define QUEUE_H

typedef struct _Queue Queue;

int queue_init(Queue *qp, int size);
int cola_llena(Queue *qp);
int cola_vacia(Queue *qp);
int cola_insertar(Queue *qp, char elem);
int cola_extraer(Queue *qp, char *ret);

#endif
