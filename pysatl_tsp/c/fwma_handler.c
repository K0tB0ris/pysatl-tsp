#include "fwma_handler.h"
#include "handler.h"
#include <stdio.h>
#include <stdlib.h>

/* Generate Fibonacci sequence for weighted moving average */
static int tsp_create_fibonacci_sequence(struct tsp_fwma_data *data) {
	struct tsp_queue *queue = data->queue;
	int tmp = queue->capacity - 1;

	// Generate Fibonacci sequence
	for (int i = 0; i < queue->capacity; i++) {
		if (data->asc == 1) {
			// Ascending order: store from start to end
			if (i < 2) {
				data->fib_sequence[i] = 1.0;
			} else {
				data->fib_sequence[i] =
				    data->fib_sequence[i - 1] + data->fib_sequence[i - 2];
			}
			data->fib_sum += data->fib_sequence[i];
		} else {
			// Descending order: store from end to start
			if (i < 2) {
				data->fib_sequence[tmp - i] = 1.0;
			} else {
				data->fib_sequence[tmp - i] = data->fib_sequence[tmp - i + 1] +
							      data->fib_sequence[tmp - i + 2];
			}
			data->fib_sum += data->fib_sequence[tmp - i];
		}
	}
	// Normalize weights to create probability distribution
	for (int i = 0; i < queue->capacity; i++) {
		data->fib_sequence[i] = data->fib_sequence[i] / data->fib_sum;
	}
	return 0;
}

/*
 * Initializes a TSP Fibonacci Weighted Moving Average data structure
 *
 * Configuration parameters:
 *
 * capacity: Maximum number of elements the structure can hold
 * asc: Ascending flag (purpose depends on implementation)
 *
 * return: Pointer to initialized structure, or NULL on failure
 *
 */
struct tsp_fwma_data *tsp_fwma_data_init(int capacity, int asc) {
	// Step 1: Allocate memory for the main structure
	struct tsp_fwma_data *obj = malloc(sizeof(struct tsp_fwma_data));
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

	// Step 3: Allocate memory for Fibonacci sequence weights
	// This array will store Fibonacci numbers used as weights in calculations
	obj->fib_sequence = malloc(sizeof(obj->fib_sequence[0]) * capacity);
	if (obj->fib_sequence == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize fibonacci sequence\n");
		// Note: Previous allocations are not freed - potential memory leak
		return NULL;
	}

	// Step 4: Initialize remaining fields
	obj->fib_sum = 0.0; // Will store sum of Fibonacci weights
	obj->asc = asc;	    // Store the ascending flag

	// Step 5: Generate the actual Fibonacci sequence
	tsp_create_fibonacci_sequence(obj);

	return obj;
}

/* Free FWMA data structure and all its components */
void tsp_free_fwma_data(struct tsp_fwma_data *q) {
	struct tsp_fwma_data *p = q;
	if (p->queue != NULL) {
		tsp_free_queue((void *)p->queue);
	}
	free(p);
}

/* Add new value to FWMA data buffer */
static int tsp_fwma_data_put(struct tsp_fwma_data *data, float value) {
	struct tsp_queue *queue = data->queue;
	queue->buffer[queue->tail] = value;
	queue->tail = (queue->tail + 1) % queue->capacity;
	return 0;
}

/* get value from FWMA data buffer */
static double tsp_fwma_data_get(struct tsp_fwma_data *data) {
	struct tsp_queue *queue = data->queue;
	double value = queue->buffer[queue->head];
	queue->head = (queue->head + 1) % queue->capacity;
	return value;
}

/*
 * Computes Fibonacci Weighted Moving Average (FWMA)
 *
 * This operator calculates a moving average where data points are weighted
 * according to Fibonacci sequence values. During the initial "warm-up" period
 * when the buffer isn't full, it returns NaN until sufficient data is collected.
 *
 * @param handler: TSP handler containing FWMA data and queue
 * @param next: Pointer to the new data value to process
 * @return: FWMA value once buffer is full, inf(None) during warm-up period
 */
double tsp_op_FWMA(struct tsp_handler *handler, void *next) {
	struct tsp_fwma_data *data = (struct tsp_fwma_data *)handler->data;
	struct tsp_queue *q = (struct tsp_queue *)data->queue;
	double *value = (double *)next;
	double weighted_sum = 0.0;
	// Phase 1: Buffer filling (warm-up period)
	// During initial calls, fill the buffer until we have enough data points
	if (q->size < q->capacity) {
		q->size++;			 // Track how many values we've collected
		q->sum += *value;		 // Maintain running sum (unused in FWMA?)
		tsp_fwma_data_put(data, *value); // Add value to circular buffer
	} else {
		// Buffer is full - normal operation, just add the new value
		tsp_fwma_data_put(data, *value);
	}

	if (q->size < q->capacity) {
		double none = 1.0 / 0.0; // Creates infinity/NaN
		return none;
	}
	// Phase 2: Calculate weighted average
	// Multiply each data point in the buffer by its corresponding Fibonacci weight
	for (int i = 0; i < q->capacity; i++) {
		weighted_sum += data->fib_sequence[i] * tsp_fwma_data_get(data);
	}
	q->head = (q->head + 1) % q->capacity;

	return weighted_sum;
}
