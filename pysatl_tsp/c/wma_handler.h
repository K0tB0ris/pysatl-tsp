#define TSP_API_START
#define TSP_API_END
#ifndef WMA_HANDLER_H
#define WMA_HANDLER_H
#include "handler.h"

TSP_API_START
/*
 * Weighted Moving Average (WMA) Data Structure
 *
 * This structure holds all necessary data for calculating weighted moving averages,
 * which use traditional linear weights.
 */
struct tsp_wma_data {
	struct tsp_queue *queue; /* Circular buffer for price data storage */
	double *weights;	 /* Pre-computed Fibonacci weights array */
	double total_weight;	 /* Sum of all Fibonacci weights for normalization */
	int asc;		 /* Weight order: 1=ascending, 0=descending */
};

struct tsp_wma_data *tsp_wma_data_init(int capacity, int asc);
void tsp_free_wma_data(struct tsp_wma_data *q);
double tsp_op_WMA(struct tsp_handler *handler, void *next);
TSP_API_END
#endif /* WMA_HANDLER_H */
