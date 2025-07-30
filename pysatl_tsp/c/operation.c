double tsp_op_addFive(struct tsp_handler *handler, void *first) {
	double *one = (double *)first;
	double sum = 0;
	sum = *one + 5.0;
	return sum;
}

double tsp_op_multFive(struct tsp_handler *handler, void *first) {
	double *one = (double *)first;
	double sum = 0;
	sum = *one * 5.0;
	return sum;
}
