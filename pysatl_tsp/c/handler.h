typedef struct _object PyObject;

/*
 * Generic handler for TSP algorithm operations
 *
 * This versatile structure supports both C and Python execution environments
 * and can operate in different modes: buffered processing and chaining processing.
 */
struct tsp_handler {
	void *data;		 // Primary data payload (queue, parameters, etc)
	void *buffer;		 // Temporary storage for computations
	int buf_start;		 // Start index for buffer operations
	int buf_end;		 // End index for buffer operations
	struct tsp_handler *src; // Source handler for pipeline
	double (*operation)(struct tsp_handler *handler, void *); // Core computation function
	PyObject *py_iter; // Python iterator object for Python integration
};

/*
 * Circular buffer queue for TSP algorithm calculations
 * Uses double precision floating-point numbers as elements
 */
struct tsp_queue {
	double *buffer; // Storage for queue elements
	int capacity;	// Max elements the queue can contain
	int head;	// Read position (oldest element)
	int tail;	// Write position (next empty slot)
	int size;	// Current element count
	double sum;	// Precomputed sum for efficient average calculations
};

struct tsp_queue *tsp_queue_init(int capacity);
void tsp_free_queue(void *q);

struct tsp_handler *tsp_init_handler(void *data, struct tsp_handler *src,
				     double (*operation)(struct tsp_handler *handler, void *),
				     void *pyobj);
void tsp_free_handler(struct tsp_handler *handler);

double *tsp_next_buffer(struct tsp_handler *handler, int capacity);
double *tsp_next_chain(struct tsp_handler *handler, int capacity);
