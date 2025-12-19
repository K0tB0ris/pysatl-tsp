#define TSP_API_START
#define TSP_API_END
#ifndef MIDPOINT_HANDLER_H
#define MIDPOINT_HANDLER_H
#include "handler.h"

TSP_API_START
struct tsp_midpoint_data {
	struct tsp_queue *queue;
	int *min_values; // Dynamic array containing discovered minimum values
	int *max_values; // Dynamic array containing discovered maximum values
	int min_head;
	int min_size;
	int max_head;
	int max_size;
};
struct tsp_midpoint_data *tsp_midpoint_data_init(int capacity);
void tsp_free_midpoint_data(struct tsp_midpoint_data *q);
double tsp_op_MIDPOINT(struct tsp_handler *handler, void *next);
TSP_API_END
#endif /* MIDPOINT_HANDLER_H */
