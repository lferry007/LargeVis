#include "Python.h"
#include "LargeVis.h"
#include "numpy/arrayobject.h"

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif


real *out_vec;
LargeVis model;
char *filename;

static PyObject *Run(PyObject *self, PyObject *args)
{
	long long out_dim = -1;
	long long n_samples = -1;
	long long n_threads = -1;
	long long n_negatives = -1;
	long long n_neighbors = -1;
	long long n_trees = -1;
	long long n_propagation = -1;
	real alpha = -1;
	real gamma = -1;
	real perplexity = -1;
	double f1, f2, f3; f1 = f2 = f3 = -1;

	if (!PyArg_ParseTuple(args, "|LLLLdLLLdd", &out_dim, &n_threads, &n_samples, &n_propagation, &f1, &n_trees, &n_negatives, &n_neighbors, &f2, &f3))
	{
		printf("Input error!\n");
		return Py_None;
	}
	alpha = f1;
	gamma = f2;
	perplexity = f3;
	model.run(out_dim, n_threads, n_samples, n_propagation, alpha, n_trees, n_negatives, n_neighbors, gamma, perplexity);
	out_vec = model.get_ans();
	PyObject* ret = PyList_New(0);
	long long n_vertices = model.get_n_vertices();
	out_dim = model.get_out_dim();
	for (long long i = 0; i < n_vertices; ++i)
	{
		PyObject *node = PyList_New(0);
		long long ll = i * out_dim;
		for (long long j = 0; j < out_dim; ++j)
			PyList_Append(node, PyFloat_FromDouble(out_vec[ll + j]));
		PyList_Append(ret, node);
	}
	return ret;
}

static PyObject *LoadFromFile(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, "s", &filename))
	{
		printf("Input error!\n");
		return Py_None;
	}
	model.load_from_file(filename);
	return Py_None;
}

static PyObject *LoadFromGraph(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, "s", &filename))
	{
		printf("Input error!\n");
		return Py_None;
	}
	model.load_from_graph(filename);
	return Py_None;
}

static PyObject *LoadFromList(PyObject *self, PyObject *args)
{
	PyObject *v;
	long long n_vertices;
	long long n_dim;
	if (!PyArg_ParseTuple(args, "O", &v))
	{
		printf("Input error!\n");
		return NULL;
	}
	n_vertices = PyList_Size(v);
	n_dim = PyList_Size(PyList_GetItem(v, 0));
	real *data = new real[n_vertices * n_dim];
	for (long long i = 0; i < n_vertices; ++i)
	{
		PyObject *vec = PyList_GetItem(v, i);
		long long ll = i * n_dim;
		if (i % 3000 == 0 || i == n_vertices - 1)
		{
			printf("Reading feature vectors %.2f%%%c", i * 100.0 / n_vertices, 13);
			fflush(stdout);
		}
		if (PyList_Size(vec) != n_dim)
		{
			printf("Input dimension error!\n");
			return Py_None;
		}
		for (long long j = 0; j < n_dim; ++j)
		{
#ifdef IS_PY3K
			real x = atof(PyBytes_AS_STRING(PyObject_Bytes(PyList_GetItem(vec, j))));
#else
			real x = atof(PyString_AsString(PyObject_Str(PyList_GetItem(vec, j))));
#endif
			data[ll + j] = x;
		}
	}
	printf("\n");
	model.load_from_data(data, n_vertices, n_dim);
	return Py_None;
}

static PyObject *LoadFromArray(PyObject *self, PyObject *args) 
{
	PyArrayObject *input;
	long long n_vertices;
	long long n_dim;

	//printf("Starting LoadFromArray\n");

	if (!PyArg_ParseTuple(args, "O", &input)) return NULL;

	if (NULL == input) return NULL;

	//printf("Got input object parsed as array\n");
	
	// Verify we have a 2D array of doubles
	if ((PyArray_NDIM(input) != 2) || (!PyArray_ISFLOAT(input))) return NULL;

	n_vertices = PyArray_DIM(input, 0);
	n_dim = PyArray_DIM(input, 1);

	//printf("Read array data as shape (%i, %i)\n", n_vertices, n_dim);

	//real *data = new real[n_vertices * n_dim];

	//printf("Allocated new data array\n", n_vertices, n_dim);

	real *indata = (real *) PyArray_DATA(input);

	// printf("Got pointer to input data\n");

	// for (long long i = 0; i < n_vertices; ++i) {
	// 	printf("Processing row %i\n", i);
	// 	for (long long j = 0; j < n_dim; ++j) {
	// 		// data[i * n_dim + j] = (real) *((real *) PyArray_GETPTR2(input, i, j));
	// 		printf("processing col %i\n", j);
	// 		data[i * n_dim + j] = indata[i * n_dim + j];
	// 	}
	// }

	//printf("Completed reading in data from numpy array\n");

	model.load_from_data(indata, n_vertices, n_dim);
	return Py_None;
}

static PyObject *SaveToFile(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, "s", &filename))
	{
		printf("Input error!\n");
		return Py_None;
	}
	model.save(filename);
	return Py_None;
}


static PyMethodDef LargeVis_methods[] =
{
	{ "run", Run, METH_VARARGS, "(All arguments are optional.\nrun(output dimension, threads number, training samples, propagations number, learning rate, rp-trees number, negative samples number, neighbors number, gamma, perplexity)\nFire up LargeVis." },
	{ "loadfile", LoadFromFile, METH_VARARGS, "loadfile(str filename)\nLoad high-dimensional feature vectors from file." },
	{ "loadgraph", LoadFromGraph, METH_VARARGS, "loadfile(str filename)\nLoad graph from file." },
	{ "loaddata", LoadFromList, METH_VARARGS, "loaddata(X)\nLoad data from list." },
	{ "loadarray", LoadFromArray, METH_VARARGS, "loadarray(X)\nLoad data from a numpy array."},
	{ "save", SaveToFile, METH_VARARGS, "save(str filename)\nSave data to file." },
	{ NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3

static int LargeVis_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int LargeVis_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}
static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "LargeVis",
        NULL,
        sizeof(struct module_state),
        LargeVis_methods,
        NULL,
        LargeVis_traverse,
        LargeVis_clear,
        NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_LargeVis(void)

#else
#define INITERROR return

PyMODINIT_FUNC
initLargeVis(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&moduledef);
#else
	PyObject *module = Py_InitModule("LargeVis", LargeVis_methods);
#endif
	if (module == NULL)
    INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("LargeVis.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}
