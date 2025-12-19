#define PY_SSIZE_T_CLEAN
#include "handler.h"
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

void setUp(void) {}    /* Is run before every test, put unit init calls here. */
void tearDown(void) {} /* Is run after every test, put unit clean-up calls here. */

double tsp_op_addFive(struct tsp_handler *handler, void *first) {
	double *one = (double *)first;
	handler->buf_start = handler->buf_start;
	double sum = 0;
	sum = *one + 5.0;
	return sum;
}

void even_numbers_buffer_call_basic_tests(PyObject *pIter, long count) {
	struct tsp_handler *handler = tsp_init_handler(NULL, NULL, tsp_op_addFive, pIter);
	double tmp = 0.0;
	double *res;
	long size = count;
	for (int i = 0; i < size / 2 + 100; i++) {
		res = tsp_next_buffer(handler, 64);
		if (i <= size / 2) {
			TEST_ASSERT_NOT_NULL_MESSAGE(
			    res, "The handler returned NULL instead of a number");
			if (res != NULL) {
				tmp = *res;
				TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
				    2 * i + 5.0, tmp, "The handler calculated incorrectly");
			}
		} else {
			TEST_ASSERT_NULL_MESSAGE(res,
						 "If the iterator is empty, it should return NULL");
		}
	}
	tsp_free_handler(handler);
}

void even_numbers_buffer_basic_test(void) {
	PyObject *pName, *pModule, *pClass, *pInstance, *pIter;

	// Add current directory to sys.path
	PyObject *sys_module = PyImport_ImportModule("sys");
	PyObject *path_list = PyObject_GetAttrString(sys_module, "path");
	PyList_Append(path_list, PyUnicode_FromString("."));
	Py_DECREF(sys_module);
	Py_DECREF(path_list);
	pName = PyUnicode_FromString("evennumbers");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != NULL) {
		pClass = PyObject_GetAttrString(pModule, "EvenNumbers");
		Py_DECREF(pModule);
		long c_arg = 1000;
		if (pClass != NULL) {
			pInstance = PyObject_CallObject(pClass, Py_BuildValue("(i)", c_arg));

			if (pInstance != NULL) {
				PyGILState_STATE gstate = PyGILState_Ensure();
				// Get Python iterator
				pIter = PyObject_GetIter(pInstance);
				if (pIter != NULL) {
					even_numbers_buffer_call_basic_tests(pIter, c_arg);
					Py_DECREF(pIter);
				} else {
					PyErr_Print();
				}
				Py_DECREF(pInstance);
				PyGILState_Release(gstate);
			} else {
				PyErr_Print();
			}

			Py_DECREF(pClass);
		} else {
			PyErr_Print();
		}
	} else {
		PyErr_Print();
	}
}

void odd_numbers_buffer_call_basic_tests(PyObject *pIter, long count) {
	struct tsp_handler *handler = tsp_init_handler(NULL, NULL, tsp_op_addFive, pIter);
	double tmp = 0.0;
	double *res;
	long size = count;
	if (count % 2 == 1) {
		size++;
	}
	for (int i = 0; i < size / 2 + 100; i++) {
		res = tsp_next_buffer(handler, 64);
		if (i < size / 2) {
			TEST_ASSERT_NOT_NULL_MESSAGE(
			    res, "The handler returned NULL instead of a number");
			if (res != NULL) {
				tmp = *res;
				TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
				    2 * i + 6.0, tmp, "The handler calculated incorrectly");
			}
		} else {
			TEST_ASSERT_NULL_MESSAGE(res,
						 "If the iterator is empty, it should return NULL");
		}
	}
	tsp_free_handler(handler);
}

void odd_numbers_buffer_basic_test(void) {
	PyObject *pName, *pModule, *pClass, *pInstance, *pIter;

	// Add current directory to sys.path
	PyObject *sys_module = PyImport_ImportModule("sys");
	PyObject *path_list = PyObject_GetAttrString(sys_module, "path");
	PyList_Append(path_list, PyUnicode_FromString("."));
	Py_DECREF(sys_module);
	Py_DECREF(path_list);
	pName = PyUnicode_FromString("oddnumbers");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != NULL) {
		pClass = PyObject_GetAttrString(pModule, "OddNumbers");
		Py_DECREF(pModule);
		long c_arg = 1001;
		if (pClass != NULL) {
			pInstance = PyObject_CallObject(pClass, Py_BuildValue("(i)", c_arg));

			if (pInstance != NULL) {
				PyGILState_STATE gstate = PyGILState_Ensure();
				// Get Python iterator
				pIter = PyObject_GetIter(pInstance);
				if (pIter != NULL) {
					odd_numbers_buffer_call_basic_tests(pIter, c_arg);
					Py_DECREF(pIter);
				} else {
					PyErr_Print();
				}
				Py_DECREF(pInstance);
				PyGILState_Release(gstate);
			} else {
				PyErr_Print();
			}

			Py_DECREF(pClass);
		} else {
			PyErr_Print();
		}
	} else {
		PyErr_Print();
	}
}

void natural_numbers_buffer_call_basic_tests(PyObject *pIter, long count) {
	struct tsp_handler *handler = tsp_init_handler(NULL, NULL, tsp_op_addFive, pIter);
	double tmp = 0.0;
	double *res;
	long size = count;
	for (int i = 0; i < size + 100; i++) {
		res = tsp_next_buffer(handler, 64);
		if (i <= size) {
			TEST_ASSERT_NOT_NULL_MESSAGE(
			    res, "The handler returned NULL instead of a number");
			if (res != NULL) {
				tmp = *res;
				TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
				    i + 5.0, tmp, "The handler calculated incorrectly");
			}
		} else {
			TEST_ASSERT_NULL_MESSAGE(res,
						 "If the iterator is empty, it should return NULL");
		}
	}
	tsp_free_handler(handler);
}

void natural_numbers_buffer_basic_test(void) {
	PyObject *pName, *pModule, *pClass, *pInstance, *pIter;

	// Add current directory to sys.path
	PyObject *sys_module = PyImport_ImportModule("sys");
	PyObject *path_list = PyObject_GetAttrString(sys_module, "path");
	PyList_Append(path_list, PyUnicode_FromString("."));
	Py_DECREF(sys_module);
	Py_DECREF(path_list);
	pName = PyUnicode_FromString("naturalnumbers");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != NULL) {
		pClass = PyObject_GetAttrString(pModule, "NaturalNumbers");
		Py_DECREF(pModule);
		long c_arg = 1000;
		if (pClass != NULL) {
			pInstance = PyObject_CallObject(pClass, Py_BuildValue("(i)", c_arg));

			if (pInstance != NULL) {
				PyGILState_STATE gstate = PyGILState_Ensure();
				// Get Python iterator
				pIter = PyObject_GetIter(pInstance);
				if (pIter != NULL) {
					natural_numbers_buffer_call_basic_tests(pIter, c_arg);
					Py_DECREF(pIter);
				} else {
					PyErr_Print();
				}
				Py_DECREF(pInstance);
				PyGILState_Release(gstate);
			} else {
				PyErr_Print();
			}
			Py_DECREF(pClass);
		} else {
			PyErr_Print();
		}
	} else {
		PyErr_Print();
	}
}

void even_numbers_pipeline_call_basic_tests(PyObject *pIter, long count) {
	struct tsp_handler *handler1 = tsp_init_handler(NULL, NULL, tsp_op_addFive, pIter);
	struct tsp_handler *handler2 = tsp_init_handler(NULL, handler1, tsp_op_addFive, NULL);
	double tmp = 0.0;
	double *res;
	long size = count;
	for (int i = 0; i < size / 2 + 100; i++) {
		res = tsp_next_chain(handler2, 64);
		if (i <= size / 2) {
			TEST_ASSERT_NOT_NULL_MESSAGE(
			    res, "The handler returned NULL instead of a number");
			if (res != NULL) {
				tmp = *res;
				TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
				    2 * i + 10.0, tmp, "The handler calculated incorrectly");
			}
		} else {
			TEST_ASSERT_NULL_MESSAGE(res,
						 "If the iterator is empty, it should return NULL");
		}
	}
	tsp_free_handler(handler1);
	tsp_free_handler(handler2);
}

void even_numbers_pipeline_basic_test(void) {
	PyObject *pName, *pModule, *pClass, *pInstance, *pIter;

	// Add current directory to sys.path
	PyObject *sys_module = PyImport_ImportModule("sys");
	PyObject *path_list = PyObject_GetAttrString(sys_module, "path");
	PyList_Append(path_list, PyUnicode_FromString("."));
	Py_DECREF(sys_module);
	Py_DECREF(path_list);
	pName = PyUnicode_FromString("evennumbers");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != NULL) {
		pClass = PyObject_GetAttrString(pModule, "EvenNumbers");
		Py_DECREF(pModule);
		long c_arg = 1000;
		if (pClass != NULL) {
			pInstance = PyObject_CallObject(pClass, Py_BuildValue("(i)", c_arg));

			if (pInstance != NULL) {
				PyGILState_STATE gstate = PyGILState_Ensure();
				// Get Python iterator
				pIter = PyObject_GetIter(pInstance);
				if (pIter != NULL) {
					even_numbers_pipeline_call_basic_tests(pIter, c_arg);
					Py_DECREF(pIter);
				} else {
					PyErr_Print();
				}
				Py_DECREF(pInstance);
				PyGILState_Release(gstate);
			} else {
				PyErr_Print();
			}

			Py_DECREF(pClass);
		} else {
			PyErr_Print();
		}
	} else {
		PyErr_Print();
	}
}

void odd_numbers_pipeline_call_basic_tests(PyObject *pIter, long count) {
	struct tsp_handler *handler1 = tsp_init_handler(NULL, NULL, tsp_op_addFive, pIter);
	struct tsp_handler *handler2 = tsp_init_handler(NULL, handler1, tsp_op_addFive, NULL);
	double tmp = 0.0;
	double *res;
	long size = count;
	if (count % 2 == 1) {
		size++;
	}
	for (int i = 0; i < size / 2 + 100; i++) {
		res = tsp_next_chain(handler2, 64);
		if (i < size / 2) {
			TEST_ASSERT_NOT_NULL_MESSAGE(
			    res, "The handler returned NULL instead of a number");
			if (res != NULL) {
				tmp = *res;
				TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
				    2 * i + 11.0, tmp, "The handler calculated incorrectly");
			}
		} else {
			TEST_ASSERT_NULL_MESSAGE(res,
						 "If the iterator is empty, it should return NULL");
		}
	}
	tsp_free_handler(handler1);
	tsp_free_handler(handler2);
}

void odd_numbers_pipeline_basic_test(void) {
	PyObject *pName, *pModule, *pClass, *pInstance, *pIter;

	// Add current directory to sys.path
	PyObject *sys_module = PyImport_ImportModule("sys");
	PyObject *path_list = PyObject_GetAttrString(sys_module, "path");
	PyList_Append(path_list, PyUnicode_FromString("."));
	Py_DECREF(sys_module);
	Py_DECREF(path_list);
	pName = PyUnicode_FromString("oddnumbers");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != NULL) {
		pClass = PyObject_GetAttrString(pModule, "OddNumbers");
		Py_DECREF(pModule);
		long c_arg = 1001;
		if (pClass != NULL) {
			pInstance = PyObject_CallObject(pClass, Py_BuildValue("(i)", c_arg));

			if (pInstance != NULL) {
				PyGILState_STATE gstate = PyGILState_Ensure();
				// Get Python iterator
				pIter = PyObject_GetIter(pInstance);
				if (pIter != NULL) {
					odd_numbers_pipeline_call_basic_tests(pIter, c_arg);
					Py_DECREF(pIter);
				} else {
					PyErr_Print();
				}
				Py_DECREF(pInstance);
				PyGILState_Release(gstate);
			} else {
				PyErr_Print();
			}

			Py_DECREF(pClass);
		} else {
			PyErr_Print();
		}
	} else {
		PyErr_Print();
	}
}

void natural_numbers_pipeline_call_basic_tests(PyObject *pIter, long count) {
	struct tsp_handler *handler1 = tsp_init_handler(NULL, NULL, tsp_op_addFive, pIter);
	struct tsp_handler *handler2 = tsp_init_handler(NULL, handler1, tsp_op_addFive, NULL);
	double tmp = 0.0;
	double *res;
	long size = count;
	for (int i = 0; i < size + 100; i++) {
		res = tsp_next_chain(handler2, 64);
		if (i <= size) {
			TEST_ASSERT_NOT_NULL_MESSAGE(
			    res, "The handler returned NULL instead of a number");
			if (res != NULL) {
				tmp = *res;
				TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(
				    i + 10.0, tmp, "The handler calculated incorrectly");
			}
		} else {
			TEST_ASSERT_NULL_MESSAGE(res,
						 "If the iterator is empty, it should return NULL");
		}
	}
	tsp_free_handler(handler1);
	tsp_free_handler(handler2);
}

void natural_numbers_pipeline_basic_test(void) {
	PyObject *pName, *pModule, *pClass, *pInstance, *pIter;

	// Add current directory to sys.path
	PyObject *sys_module = PyImport_ImportModule("sys");
	PyObject *path_list = PyObject_GetAttrString(sys_module, "path");
	PyList_Append(path_list, PyUnicode_FromString("."));
	Py_DECREF(sys_module);
	Py_DECREF(path_list);
	pName = PyUnicode_FromString("naturalnumbers");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != NULL) {
		pClass = PyObject_GetAttrString(pModule, "NaturalNumbers");
		Py_DECREF(pModule);
		long c_arg = 1000;
		if (pClass != NULL) {
			pInstance = PyObject_CallObject(pClass, Py_BuildValue("(i)", c_arg));

			if (pInstance != NULL) {
				PyGILState_STATE gstate = PyGILState_Ensure();
				// Get Python iterator
				pIter = PyObject_GetIter(pInstance);
				if (pIter != NULL) {
					natural_numbers_pipeline_call_basic_tests(pIter, c_arg);
					Py_DECREF(pIter);
				} else {
					PyErr_Print();
				}
				Py_DECREF(pInstance);
				PyGILState_Release(gstate);
			} else {
				PyErr_Print();
			}
			Py_DECREF(pClass);
		} else {
			PyErr_Print();
		}
	} else {
		PyErr_Print();
	}
}

void null_handler_test(void) {
	struct tsp_handler *handler = NULL;
	double *res;
	res = tsp_next_buffer(handler, 64);
	TEST_ASSERT_NULL_MESSAGE(res, "If the handler is NULL, then NULL must be returned");
	res = tsp_next_chain(handler, 64);
	TEST_ASSERT_NULL_MESSAGE(res, "If the handler is NULL, then NULL must be returned");
}

void null_python_iterator_test(void) {
	struct tsp_handler *handler = tsp_init_handler(NULL, NULL, tsp_op_addFive, NULL);
	double *res;

	res = tsp_next_buffer(handler, 64);
	TEST_ASSERT_NULL_MESSAGE(res, "Should return NULL, if no Python iterator is declared");
	tsp_free_handler(handler);

	handler = tsp_init_handler(NULL, NULL, tsp_op_addFive, NULL);
	res = tsp_next_chain(handler, 64);
	TEST_ASSERT_NULL_MESSAGE(res, "Should return NULL, if no Python iterator is declared");
	tsp_free_handler(handler);
}

void null_operation_call_test(PyObject *pIter) {
	struct tsp_handler *handler1 = tsp_init_handler(NULL, NULL, NULL, pIter);
	struct tsp_handler *handler2 = tsp_init_handler(NULL, handler1, tsp_op_addFive, NULL);
	double *res;
	res = tsp_next_buffer(handler1, 64);
	TEST_ASSERT_NULL_MESSAGE(res, "Should return NULL, if there is no operation");

	res = tsp_next_chain(handler2, 64);
	TEST_ASSERT_NULL_MESSAGE(res, "Should return NULL, if there is no operation");

	tsp_free_handler(handler1);
	tsp_free_handler(handler2);
}

void null_operation_test(void) {
	PyObject *pName, *pModule, *pClass, *pInstance, *pIter;

	// Add current directory to sys.path
	PyObject *sys_module = PyImport_ImportModule("sys");
	PyObject *path_list = PyObject_GetAttrString(sys_module, "path");
	PyList_Append(path_list, PyUnicode_FromString("."));
	Py_DECREF(sys_module);
	Py_DECREF(path_list);
	pName = PyUnicode_FromString("naturalnumbers");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (pModule != NULL) {
		pClass = PyObject_GetAttrString(pModule, "NaturalNumbers");
		Py_DECREF(pModule);
		if (pClass != NULL) {
			pInstance = PyObject_CallObject(pClass, Py_BuildValue("(i)", 10));

			if (pInstance != NULL) {
				PyGILState_STATE gstate = PyGILState_Ensure();
				// Get Python iterator
				pIter = PyObject_GetIter(pInstance);
				if (pIter != NULL) {
					null_operation_call_test(pIter);
					Py_DECREF(pIter);
				} else {
					PyErr_Print();
				}
				Py_DECREF(pInstance);
				PyGILState_Release(gstate);
			} else {
				PyErr_Print();
			}
			Py_DECREF(pClass);
		} else {
			PyErr_Print();
		}
	} else {
		PyErr_Print();
	}
}

int main() {
	UNITY_BEGIN();
	Py_Initialize();
	RUN_TEST(even_numbers_buffer_basic_test);
	RUN_TEST(odd_numbers_buffer_basic_test);
	RUN_TEST(natural_numbers_buffer_basic_test);

	RUN_TEST(even_numbers_pipeline_basic_test);
	RUN_TEST(odd_numbers_pipeline_basic_test);
	RUN_TEST(natural_numbers_pipeline_basic_test);

	RUN_TEST(null_handler_test);
	RUN_TEST(null_python_iterator_test);
	RUN_TEST(null_operation_test);

	Py_Finalize();
	return UNITY_END();
}
