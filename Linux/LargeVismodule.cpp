#include "Python.h"
#include "LargeVis.h"

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
			real x = atof(PyString_AsString(PyObject_Str(PyList_GetItem(vec, j))));
			data[ll + j] = x;
		}
	}
	printf("\n");
	model.load_from_data(data, n_vertices, n_dim);
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

static PyMethodDef PyExtMethods[] =
{
	{ "run", Run, METH_VARARGS, "(All arguments are optional.\nrun(output dimension, threads number, training samples, propagations number, learning rate, rp-trees number, negative samples number, neighbors number, gamma, perplexity)\nFire up LargeVis." },
	{ "loadfile", LoadFromFile, METH_VARARGS, "loadfile(str filename)\nLoad high-dimensional feature vectors from file." },
	{ "loadgraph", LoadFromGraph, METH_VARARGS, "loadfile(str filename)\nLoad graph from file." },
	{ "loaddata", LoadFromList, METH_VARARGS, "loaddata(X)\nLoad data from list." },
	{ "save", SaveToFile, METH_VARARGS, "save(str filename)\nSave data to file." },
	{ NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initLargeVis()
{
	printf("LargeVis successfully imported!\n");
	Py_InitModule("LargeVis", PyExtMethods);
}
