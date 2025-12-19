#include "midpoint_handler.h"
#include "handler.h"
#include <stdio.h>
#include <stdlib.h>

struct tsp_midpoint_data *tsp_midpoint_data_init(int capacity) {
	struct tsp_midpoint_data *obj = malloc(sizeof(struct tsp_midpoint_data));
	if (obj == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize ema's data\n");
		return NULL;
	}
	obj->queue = tsp_queue_init(capacity);
	if (obj->queue == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize queue\n");
		free(obj);
		return NULL;
	}
	obj->min_values = malloc(capacity * sizeof(obj->min_values[0]));
	if (!obj->min_values) {
		fprintf(stderr, "Could not allocate memory for MinResult\n");
		tsp_free_queue((void *)obj->queue);
		free(obj);
		return NULL;
	}
	obj->max_values = malloc(capacity * sizeof(obj->max_values[0]));
	if (!obj->max_values) {
		fprintf(stderr, "Could not allocate memory for MaxResult\n");
		free(obj->min_values);
		tsp_free_queue((void *)obj->queue);
		free(obj);
		return NULL;
	}
	for (int i = 0; i < capacity; i++) {
		obj->min_values[i] = 0;
		obj->max_values[i] = 0;
	}
	obj->min_size = 0;
	obj->max_size = 0;
	obj->min_head = 0;
	obj->max_head = 0;
	return obj;
}

void tsp_free_midpoint_data(struct tsp_midpoint_data *q) {
	struct tsp_midpoint_data *obj = q;
	if (obj != NULL) {
		if (obj->min_values != NULL) {
			free(obj->min_values);
		}
		if (obj->max_values != NULL) {
			free(obj->max_values);
		}
		if (obj->queue != NULL) {
			tsp_free_queue((void *)obj->queue);
		}
		free(obj);
	}
}

static int tsp_midpoint_put(struct tsp_midpoint_data *data, double value) {
	struct tsp_queue *queue = data->queue;
	if ((data->min_size == 0) && (data->max_size == 0)) {
		data->min_values[data->min_head + data->min_size] = queue->tail;
		data->max_values[data->max_head + data->max_size] = queue->tail;
		data->min_size++;
		data->max_size++;
		queue->buffer[queue->tail] = value;
		queue->tail = (queue->tail + 1) % queue->capacity;
		return 0;
	}
	if (value != value) {
		data->max_size = 1;
		data->min_size = 1;
		data->max_head = 0;
		data->min_head = 0;
		data->min_values[0] = queue->tail;
		data->max_values[0] = queue->tail;
		queue->buffer[queue->tail] = value;
		queue->tail = (queue->tail + 1) % queue->capacity;
	}

	while (data->min_size != 0) {
		int position = (data->min_head + data->min_size - 1) % queue->capacity;
		if (queue->buffer[data->min_values[position]] > value) {
			data->min_size--;
		} else {
			if (data->min_values[data->min_head] == queue->tail) {
				data->min_head++;
				data->min_head = data->min_head % queue->capacity;
			}
			data->min_values[(position + 1) % queue->capacity] = queue->tail;
			if (data->min_size < queue->capacity) {
				data->min_size++;
			}
			break;
		}
		if (data->min_size == 0) {
			data->min_values[data->min_head] = queue->tail;
			data->min_size++;
			break;
		}
	}

	while (data->max_size != 0) {
		int position = (data->max_head + data->max_size - 1) % queue->capacity;
		if (queue->buffer[data->max_values[position]] < value) {
			data->max_size--;
		} else {
			if (data->max_values[data->max_head] == queue->tail) {
				data->max_head++;
				data->max_head = data->max_head % queue->capacity;
			}
			data->max_values[(position + 1) % queue->capacity] = queue->tail;
			if (data->max_size < queue->capacity) {
				data->max_size++;
			}
			break;
		}
		if (data->max_size == 0) {
			data->max_values[data->max_head] = queue->tail;
			data->max_size++;
			break;
		}
	}

	queue->buffer[queue->tail] = value;
	queue->tail = (queue->tail + 1) % queue->capacity;
	return 0;
}

double tsp_op_MIDPOINT(struct tsp_handler *handler, void *next) {
	struct tsp_midpoint_data *data = (struct tsp_midpoint_data *)handler->data;
	double *value = (double *)next;

	tsp_midpoint_put(data, *value);
	if (data->queue->size < data->queue->capacity - 1) {
		data->queue->size++;
		double none = 0.0 / 0.0;
		return none;
	}

	double min = data->queue->buffer[data->min_values[data->min_head]] / 2;
	double max = data->queue->buffer[data->max_values[data->max_head]] / 2;
	return min + max;
}
