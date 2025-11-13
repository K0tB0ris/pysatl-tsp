#include "wma_handler.h"
#include "handler.h"
#include <stdio.h>
#include <stdlib.h>

/* Generate traditional linear weights for weighted moving average */
static int tsp_create_weights(struct tsp_wma_data *data) {
	struct tsp_queue *queue = data->queue;

	// Generate weights
	for (int i = 0; i < queue->capacity; i++) {
		if (data->asc == 1) {
			// Ascending order: store from start to end
			data->weights[i] = i + 1;
		} else {
			// Descending order: store from end to start
			data->weights[queue->capacity - i - 1] = i + 1;
		}
	}

	// Normalize weights to create probability distribution
	data->total_weight = queue->capacity * (queue->capacity + 1) / 2;
	for (int i = 0; i < queue->capacity; i++) {
		data->weights[i] = data->weights[i] / data->total_weight;
	}

	return 0;
}

struct tsp_wma_data *tsp_wma_data_init(int capacity, int asc) {
	// Step 1: Allocate memory for the main structure
	struct tsp_wma_data *obj = malloc(sizeof(struct tsp_wma_data));
	if (obj == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize ema's data\n");
		return NULL;
	}

	// Step 2: Initialize the data queue
	// The queue will store the actual data points for moving average calculation
	obj->queue = tsp_queue_init(capacity);
	if (obj->queue == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize queue\n");
		// Note: obj is not freed here - potential memory leak
		return NULL;
	}

	// Step 3: Allocate memory for weights
	// This array will store weights used in calculations
	obj->weights = malloc(sizeof(obj->weights[0]) * capacity);
	if (obj->weights == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize weights\n");
		// Note: Previous allocations are not freed - potential memory leak
		return NULL;
	}

	// Step 4: Initialize remaining fields
	obj->total_weight = 0.0; // Will store sum of Fibonacci weights
	obj->asc = asc;		 // Store the ascending flag

	// Step 5: Generate the actual weights
	tsp_create_weights(obj);

	return obj;
}

/* Free WMA data structure and all its components */
void tsp_free_wma_data(struct tsp_wma_data *q) {
	struct tsp_wma_data *p = q;
	if (p->queue != NULL) {
		tsp_free_queue((void *)p->queue);
	}
	free(p);
}

/* Add new value to WMA data buffer */
static int tsp_wma_data_put(struct tsp_wma_data *data, float value) {
	struct tsp_queue *queue = data->queue;
	queue->buffer[queue->tail] = value;
	queue->tail = (queue->tail + 1) % queue->capacity;
	return 0;
}

/* get value from WMA data buffer */
static double tsp_wma_data_get(struct tsp_wma_data *data) {
	struct tsp_queue *queue = data->queue;
	double value = queue->buffer[queue->head];
	queue->head = (queue->head + 1) % queue->capacity;
	return value;
}

/*
 * Computes Weighted Moving Average (WMA)
 *
 * This operator calculates a moving average where data points are weighted.
 * During the initial "warm-up" period when the buffer isn't full, it returns Inf
 * until sufficient data is collected.
 *
 * @param handler: TSP handler containing WMA data and queue
 * @param next: Pointer to the new data value to process
 * @return: WMA value once buffer is full, Inf(None) during warm-up period
 */
double tsp_op_WMA(struct tsp_handler *handler, void *next) {
	struct tsp_wma_data *data = (struct tsp_wma_data *)handler->data;
	struct tsp_queue *q = (struct tsp_queue *)data->queue;
	double *value = (double *)next;
	double weighted_sum = 0.0;
	// Phase 1: Buffer filling (warm-up period)
	// During initial calls, fill the buffer until we have enough data points
	if (q->size < q->capacity) {
		q->size++;			// Track how many values we've collected
		tsp_wma_data_put(data, *value); // Add value to circular buffer
	} else {
		// Buffer is full - normal operation, just add the new value
		tsp_wma_data_put(data, *value);
	}

	if (q->size < q->capacity) {
		double none = 1.0 / 0.0; // Creates infinity/NaN
		return none;
	}
	// Phase 2: Calculate weighted average
	// Multiply each data point in the buffer by its corresponding weight
	for (int i = 0; i < q->capacity; i++) {
		weighted_sum += data->weights[i] * tsp_wma_data_get(data);
	}
	q->head = (q->head + 1) % q->capacity;

	return weighted_sum;
}
