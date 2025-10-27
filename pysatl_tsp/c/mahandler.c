#include "handler.h"
#include <stdio.h>
#include <stdlib.h>
#include "mahandler.h"

/*
 * Circular queue insertion operation
 * Adds value to tail and advances tail pointer with wrap-around
 */
static int tsp_queue_put(struct tsp_queue *q, float value) {
	q->buffer[q->tail] = value;
	q->tail = (q->tail + 1) % q->capacity;
	return 0;
}

/*
 * Circular queue extraction operation
 * Retrieves value from head and advances head pointer with wrap-around
 */
static double tsp_queue_get(struct tsp_queue *q) {
	double value = q->buffer[q->head];
	q->head = (q->head + 1) % q->capacity;
	return value;
}

/*
 * Moving Average operation
 *
 * Implements a moving average algorithm by maintaining a running sum
 * Efficiently maintains running average using circular buffer
 *
 * Behavior phases:
 * 1. Warm-up: Queue fills until reaching capacity (increasing window size)
 * 2. Steady-state: Window slides, maintaining fixed size
 */
double tsp_op_MA(struct tsp_handler *handler, void *next) {
	struct tsp_queue *q = (struct tsp_queue *)handler->data;
	double *value = (double *)next;

	if (q->size < q->capacity) {
		// Initial filling phase - queue not yet at capacity
		q->size++;
		q->sum += *value;
		tsp_queue_put(q, *value);
	} else {
		// Queue is full - replace oldest value
		double old = tsp_queue_get(q);
		tsp_queue_put(q, *value);
		q->sum += (*value - old); // Efficient sum update
	}

	double ma = q->sum / q->size;
	return ma;
}
