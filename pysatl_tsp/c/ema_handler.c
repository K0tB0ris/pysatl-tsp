#include "ema_handler.h"
#include "handler.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * EMA Data Structure Factory Function
 *
 * Configuration parameters:
 *
 * capacity - Determines:
 *   - SMA window size (if sma enabled)
 *   - Queue buffer size
 *
 * sma - Initialization mode:
 *   - 1: Use SMA of first N values to initialize EMA (smoother start)
 *   - 0: Start EMA immediately with first value (more responsive)
 *
 * alpha - Smoothing factor customization:
 *   - NULL: Use default 2/(capacity+1)
 *   - Custom: Use provided alpha value
 *
 * adjust - Bias correction:
 * When adjust=True, uses an adjusted weighting method that gives more weight to recent
 * observations.
 */
struct tsp_ema_data *tsp_ema_data_init(int capacity, int sma, double alpha, int adjust) {
	struct tsp_ema_data *obj = malloc(sizeof(struct tsp_ema_data));
	if (obj == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize ema's data\n");
		return NULL;
	}
	obj->queue = tsp_queue_init(capacity);
	if (obj->queue == NULL) {
		fprintf(stderr, "Could not allocate memory to initialize queue\n");
		return NULL;
	}

	obj->sma = sma;
	obj->adjust = adjust;
	obj->alpha = alpha;

	obj->ema_numerator = 0;
	obj->ema_denominator = 0;
	return obj;
}

void tsp_free_ema_data(void *q) {
	struct tsp_ema_data *p = (struct tsp_ema_data *)q;
	if (p->queue != NULL) {
		tsp_free_queue((void *)p->queue);
	}
	free(p);
}

/*
 * Add value to EMA queue during SMA warm-up period
 */
static int tsp_ema_data_put(struct tsp_ema_data *data, float value) {
	struct tsp_queue *queue = data->queue;
	queue->buffer[queue->tail] = value;
	queue->tail = (queue->tail + 1) % queue->capacity;
	return 0;
}

/*
 * Reset queue state for new EMA calculation cycle
 * Note that tsp_clear_queue doesn't free the buffer, just resets counters
 */
static void tsp_clear_queue(struct tsp_queue *queue) {
	queue->head = 0;
	queue->tail = 0;
	queue->sum = 0;
}

/*
 * Updates the Exponential Moving Average state with a new value
 *
 * Manages the three operational states:
 *
 * STATE 1: SMA WARM-UP (data->sma != 0 && queue not full)
 *   - Collect data points in queue
 *   - Calculate running sum
 *   - Return early (wait for full queue)
 *
 * STATE 2: SMA→EMA TRANSITION (data->sma != 0 && queue full)
 *   - Calculate final SMA value
 *   - Clear queue (no longer needed)
 *   - Initialize EMA with SMA value
 *   - Switch to EMA mode (data->sma = 0)
 *
 * STATE 3: EMA STEADY-STATE (data->sma == 0)
 *   - Update EMA using exponential smoothing
 *   - Support both biased and unbiased EMA variants
 */
static int tsp_update_state(struct tsp_ema_data *data, double *value) {

	// Maintain queue for SMA phase or reference
	if (value != NULL) {
		if (data->queue->size < data->queue->capacity) {
			// Growing window phase
			data->queue->size++;
			data->queue->sum += *value;
			tsp_ema_data_put(data, *value);
		}
	}

	// Wait until queue is full before transitioning from SMA to EMA
	if ((data->sma != 0) && (data->queue->size < data->queue->capacity)) {
		return 0;
	}

	// State transition and EMA update logic
	if (data->sma != 0) {
		// Transition from SMA to EMA
		if (data->queue->size != 0) {
			double sma_value = data->queue->sum / data->queue->size;
			tsp_clear_queue(data->queue);
			data->ema_numerator = sma_value;
			data->ema_denominator = 1;
		}
		data->sma = 0; // Enter EMA mode
	} else if ((data->adjust != 0) && (value != NULL)) {
		// Unbiased EMA (accounts for limited history)
		data->ema_numerator = (1 - data->alpha) * data->ema_numerator + *value;
		data->ema_denominator = (1 - data->alpha) * data->ema_denominator + 1;
	} else if ((data->adjust == 0) && (value != NULL)) {
		// Standard EMA (assumes infinite history)
		if (data->ema_denominator != 0.0) {
			data->ema_numerator =
			    (1 - data->alpha) * data->ema_numerator + data->alpha * *value;
		} else {
			data->ema_numerator = *value; // First value initialization
		}
		data->ema_denominator = 1.0;
	}
	return 0;
}

/*
 * Exponential Moving Average (EMA) operation for TSP handler
 *
 * Processes a new value through the EMA state machine and returns the
 * current EMA value. Handles both SMA initialization phase and steady-state
 * EMA calculation with proper error checking for division by zero.
 *
 * NOTE: The infinity return value serves as a sentinel indicating
 * "value not yet available" rather than a mathematical error.
 */
double tsp_op_EMA(struct tsp_handler *handler, void *next) {
	struct tsp_ema_data *data = (struct tsp_ema_data *)handler->data;
	double *value = (double *)next;

	// Update EMA state machine with new value
	tsp_update_state(data, value);

	// Initialize with infinity to indicate "not available"
	double ma = 1.0 / 0.0; // Positive infinity

	// Only compute EMA if we have valid denominator (sufficient data)
	if (data->ema_denominator != 0) {
		ma = data->ema_numerator / data->ema_denominator;
	}

	return ma; // Check for inf in Python
}
