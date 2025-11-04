#define TSP_API_START
#define TSP_API_END
#ifndef OPERATION_H
#define OPERATION_H
#include "handler.h"
TSP_API_START
double tsp_op_addFive(struct tsp_handler *handler, void *first);
double tsp_op_multFive(struct tsp_handler *handler, void *first);
TSP_API_END
#endif /* OPERATION_H */
