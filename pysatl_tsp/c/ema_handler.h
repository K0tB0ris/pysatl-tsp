#define TSP_API_START
#define TSP_API_END
#ifndef EMA_HANDLER_H
#define EMA_HANDLER_H
#include "handler.h"

TSP_API_START
/*
 *
 * Data structure for Exponential Moving Average (EMA) calculations
 * with support for both simple and exponential moving averages
 *
 * This structure combines a traditional SMA (Simple Moving Average)
 * with an EMA calculation, allowing for hybrid approaches or smooth
 * transitions between averaging methods commonly used in TSP algorithms.
 */
struct tsp_ema_data {
	struct tsp_queue *queue; // Buffer for data values (mainly for SMA phase)
	int sma;		 // Whether to use SMA initialization (true/false)
	int adjust;		 // Scaling factor for unbiased EMA estimation
	double alpha;		 // Smoothing constant - typically 2/(N+1) for N-period EMA
	double ema_numerator;	 // Current EMA value before normalization
	double ema_denominator;	 // Cumulative weight sum for proper normalization
};
struct tsp_ema_data *tsp_ema_data_init(int capacity, int sma, double alpha, int adjust);
void tsp_free_ema_data(struct tsp_ema_data *q);
double tsp_op_EMA(struct tsp_handler *handler, void *next);
TSP_API_END
#endif /* EMA_HANDLER_H */
