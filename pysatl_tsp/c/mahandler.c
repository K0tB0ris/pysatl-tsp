#include "handler.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* init circular queue */

struct tsp_queue *tsp_queue_init(int capacity) {
	struct tsp_queue *obj = malloc(sizeof(struct tsp_queue));
	if (obj == NULL) {
		return NULL;
	}
	obj->buffer = malloc(capacity * sizeof(obj->buffer[0]));
	if (!obj->buffer) {
		return NULL;
	}
	obj->capacity = capacity;
	obj->head = 0;
	obj->tail = 0;
	obj->size = 0;
	obj->sum = 0;
	return obj;
}

static int tsp_queue_put(struct tsp_queue *q, float value) {
	q->buffer[q->tail] = value;
	q->tail = (q->tail + 1) % q->capacity;
	return 0;
}

static double tsp_queue_get(struct tsp_queue *q) {
	double value = q->buffer[q->head];
	q->head = (q->head + 1) % q->capacity;
	return value;
}

void tsp_free_queue(void *q) {
	struct tsp_queue *p = (struct tsp_queue *)q;
	if (p->buffer != NULL) {
		free(p->buffer);
	}
	free(p);
}

double tsp_op_MA(struct tsp_handler *handler, void *first) {
	struct tsp_queue *q = (struct tsp_queue *)handler->data;
	double *value = (double *)first;
	if (q->size < q->capacity) {
		q->size++;
		q->sum += *value;
		tsp_queue_put(q, *value);
	} else {
		double old = tsp_queue_get(q);
		tsp_queue_put(q, *value);
		q->sum += (*value - old);
	}
	double ma = q->sum / q->size;
	return ma;
}
