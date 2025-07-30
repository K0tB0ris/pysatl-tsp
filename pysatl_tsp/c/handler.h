typedef struct _object PyObject;

struct tsp_handler {
	void *data;
	void *buffer;
	int buf_start;
	int buf_end;
	struct tsp_handler *src;
	double (*operation)(struct tsp_handler *handler, void *);
	PyObject *py_iter;
};

struct tsp_queue {
	double *buffer;
	int capacity;
	int head;
	int tail;
	int size;
	double sum;
};

struct tsp_queue *tsp_queue_init(int capacity);
void tsp_free_queue(void *q);
double tsp_op_MA(struct tsp_handler *handler, void *first);

struct tsp_handler *tsp_init_handler(void *data, struct tsp_handler *src,
				     double (*operation)(struct tsp_handler *handler, void *),
				     void *pyobj);
void tsp_free_handler(struct tsp_handler *handler);
double *tsp_next_buffer(struct tsp_handler *handler, int capacity);
double *tsp_next_chain(struct tsp_handler *handler, int capacity);
