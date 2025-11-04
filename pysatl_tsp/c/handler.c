#define PY_SSIZE_T_CLEAN
#include "handler.h"
#include <Python.h>
#include <stdio.h>

/*
 * Creates and initializes a TSP handler with given components
 * See also handler.h
 */
struct tsp_handler *tsp_init_handler(void *data, struct tsp_handler *src,
				     double (*operation)(struct tsp_handler *handler, void *),
				     void *pyobj) {
	struct tsp_handler *obj = malloc(sizeof(struct tsp_handler));
	if (obj == NULL) {
		fprintf(stderr, "Could not allocate memory for Handler \n");
		return NULL;
	}
	obj->data = data;
	obj->operation = operation;
	obj->py_iter = (PyObject *)pyobj;
	obj->src = src;
	obj->buf_start = 0;
	obj->buf_end = 0;
	obj->buffer = NULL;
	return obj;
}

void tsp_free_handler(struct tsp_handler *handler) {
	if (handler->buffer != NULL) {
		free(handler->buffer);
	}
	free(handler);
}

/* Init circular queue
 * See also handler.h
 */
struct tsp_queue *tsp_queue_init(int capacity) {
	struct tsp_queue *obj = malloc(sizeof(struct tsp_queue));
	if (obj == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize Queue\n");
		return NULL;
	}
	obj->buffer = malloc(capacity * sizeof(obj->buffer[0]));
	if (!obj->buffer) {
		fprintf(stderr, "Could not allocate memory for Queue\n");
		return NULL;
	}
	obj->capacity = capacity;
	obj->head = 0;
	obj->tail = 0;
	obj->size = 0;
	obj->sum = 0;
	return obj;
}

void tsp_free_queue(void *q) {
	struct tsp_queue *p = (struct tsp_queue *)q;
	if (p->buffer != NULL) {
		free(p->buffer);
	}
	free(p);
}

/* tsp_next_buffer apply operation to the next element from the iterator */
double *tsp_next_buffer(struct tsp_handler *handler, int capacity) {
	// check handler existence
	if (handler == NULL) {
		fprintf(stderr, "Handler pointer is NULL \n");
		return NULL;
	}

	double *res = NULL;
	// return next element, if buffer is not empty
	if (handler->buf_start != handler->buf_end) {
		res = (double *)handler->buffer;
		return &res[handler->buf_start++];
	}

	// create buffer, if it doesn't exist
	if (handler->buffer == NULL) {
		handler->buffer = (void *)malloc(capacity * sizeof(double));
	}

	// Setting up future work with Python iterator
	PyGILState_STATE gstate = PyGILState_Ensure();
	PyObject *pIterator = handler->py_iter;
	Py_INCREF(pIterator);
	PyObject *pItem;

	// Get elements from iterator into buffer
	res = (double *)handler->buffer;
	handler->buf_start = 0;
	handler->buf_end = 0;
	for (int j = 0; j < capacity; j++) {
		if ((pItem = PyIter_Next(pIterator)) != NULL) {
			double tmp = PyFloat_AsDouble(pItem);
			tmp = handler->operation(handler, (void *)&tmp);
			res[handler->buf_end++] = tmp;
			Py_DECREF(pItem);
		} else {
			break;
		}
	}
	Py_DECREF(pIterator);
	PyGILState_Release(gstate);

	// return NULL, if we don't get elements from iterator
	if (handler->buf_start == handler->buf_end) {
		return NULL;
	}
	return &res[handler->buf_start++];
}

/* tsp_next_chain implements a chain of operations
 * Follows the chain until it will meet hadnler with source = NULL
 * And then sequentially applies operations to the data
 */
double *tsp_next_chain(struct tsp_handler *handler, int capacity) {
	if (handler->src == NULL) {
		// Find handler(NULL, float)
		return tsp_next_buffer(handler, capacity);
	} else {
		// create buffer, if it doesn't exist
		if (handler->buffer == NULL) {
			handler->buffer = (void *)malloc(capacity * sizeof(double));
		}

		// return next element, if buffer is not empty
		double *res = (double *)handler->buffer;
		if (handler->buf_start != handler->buf_end) {
			return &res[handler->buf_start++];
		}

		// Apply operation to the previous results
		handler->buf_start = 0;
		handler->buf_end = 0;
		for (int i = 0; i < capacity; i++) {
			double *prev = tsp_next_chain(handler->src, capacity);
			if (prev != NULL) {
				double tmp = prev[0];
				tmp = handler->operation(handler, (void *)&tmp);
				res[handler->buf_end++] = tmp;
			} else {
				break;
			}
		}

		// return NULL, if we don't have elements in buffer
		if (handler->buf_start == handler->buf_end) {
			return NULL;
		}
		return &res[handler->buf_start++];
	}
}
