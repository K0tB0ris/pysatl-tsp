#define TSP_API_START
#define TSP_API_END
#ifndef FWMA_HANDLER_H
#define FWMA_HANDLER_H
#include "handler.h"

TSP_API_START
/*
 * Fibonacci Weighted Moving Average (FWMA) Data Structure
 *
 * This structure holds all necessary data for calculating Fibonacci-weighted
 * moving averages, which use Fibonacci sequence numbers as weights instead
 * of traditional linear weights.
 */
struct tsp_fwma_data {
	struct tsp_queue *queue; /* Circular buffer for price data storage */
	double *fib_sequence;	 /* Pre-computed Fibonacci weights array */
	double fib_sum;		 /* Sum of all Fibonacci weights for normalization */
	int asc;		 /* Weight order: 1=ascending, 0=descending */
};

struct tsp_fwma_data *tsp_fwma_data_init(int capacity, int asc);
void tsp_free_fwma_data(struct tsp_fwma_data *q);
double tsp_op_FWMA(struct tsp_handler *handler, void *next);
TSP_API_END
#endif /* FWMA_HANDLER_H */
