#include <stdint.h>

#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include "convolve.h"

static char convolve_docstring[] = "Module for fast kernel convolution";

static PyObject* convolve_convolve(PyObject* self, PyObject* args);
static PyObject* convolve_convolve_id(PyObject* self, PyObject* args);
static PyObject* convolve_convolve_sobel(PyObject* self, PyObject* args);
static PyObject* convolve_convolve_gaussian(PyObject* self, PyObject* args);

static PyMethodDef convolve_methods[] = {
    {"convolve", convolve_convolve, METH_VARARGS, NULL},
    {"convolve_id", convolve_convolve_id, METH_VARARGS, NULL},
    {"convolve_sobel", convolve_convolve_sobel, METH_VARARGS, NULL},
    {"convolve_gaussian", convolve_convolve_gaussian, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef convolve_module = {
       PyModuleDef_HEAD_INIT,
       "pi_rtvp.convolve",  // name of module
       convolve_docstring,  // module documentation, may be NULL
       -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
       convolve_methods
};

PyMODINIT_FUNC PyInit_convolve(void)
{
    PyObject* m = PyModule_Create(&convolve_module);
    if (m == NULL)
        return NULL;
    import_array();
    return m;
}

static PyObject* convolve_convolve(PyObject* self, PyObject* args)
{
    int size;
    PyObject* chunk_obj;
    PyObject* kernel_obj;
    if (!PyArg_ParseTuple(args, "OOi", &chunk_obj, &kernel_obj, &size)) {
        return NULL;
    }
    PyObject* chunk_array = PyArray_FROM_OTF(chunk_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_array = PyArray_FROM_OTF(kernel_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);

    if (chunk_array == NULL || kernel_array == NULL) {
        Py_XDECREF(chunk_array);
        Py_XDECREF(kernel_array);
        return NULL;
    }

    uint8_t* chunk = (uint8_t*)PyArray_DATA((PyArrayObject*)chunk_array);
    uint8_t* kernel = (uint8_t*)PyArray_DATA((PyArrayObject*)kernel_array);
    
    uint8_t res = convolve(chunk, kernel, size);

    Py_DECREF(chunk_array);
    Py_DECREF(kernel_array);

    PyObject* out = Py_BuildValue("B", res);
    return out;
}

static PyObject* convolve_convolve_id(PyObject* self, PyObject* args)
{
    int size;
    PyObject* chunk_obj;
    if (!PyArg_ParseTuple(args, "Oi", &chunk_obj, &size)) {
        return NULL;
    }
    PyObject* chunk_array = PyArray_FROM_OTF(chunk_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);

    if (chunk_array == NULL) {
        Py_XDECREF(chunk_array);
        return NULL;
    }

    uint8_t* chunk = (uint8_t*)PyArray_DATA((PyArrayObject*)chunk_array);
    
    uint8_t res = convolve_id(chunk, size);

    Py_DECREF(chunk_array);

    PyObject* out = Py_BuildValue("B", res);
    return out;
}

static PyObject* convolve_convolve_sobel(PyObject* self, PyObject* args)
{
    int size;
    PyObject* chunk_obj;
    PyObject* kernel_x_obj;
    PyObject* kernel_y_obj;
    if (!PyArg_ParseTuple(args, "OOOi", &chunk_obj,
                          &kernel_x_obj, &kernel_y_obj, &size)) {
        return NULL;
    }
    PyObject* chunk_array = PyArray_FROM_OTF(chunk_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_x_array = PyArray_FROM_OTF(kernel_x_obj, NPY_INT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_y_array = PyArray_FROM_OTF(kernel_y_obj, NPY_INT8, NPY_ARRAY_IN_ARRAY);

    if (chunk_array == NULL || kernel_x_array == NULL || kernel_y_array == NULL) {
        Py_XDECREF(chunk_array);
        Py_XDECREF(kernel_x_array);
        Py_XDECREF(kernel_y_array);
        return NULL;
    }

    uint8_t* chunk = (uint8_t*)PyArray_DATA((PyArrayObject*)chunk_array);
    int8_t* kernel_x = (int8_t*)PyArray_DATA((PyArrayObject*)kernel_x_array);
    int8_t* kernel_y = (int8_t*)PyArray_DATA((PyArrayObject*)kernel_y_array);
    
    uint8_t res = convolve_sobel(chunk, kernel_x, kernel_y, size);

    Py_DECREF(chunk_array);
    Py_DECREF(kernel_x_array);
    Py_DECREF(kernel_y_array);

    PyObject* out = Py_BuildValue("B", res);
    return out;
}

static PyObject* convolve_convolve_gaussian(PyObject* self, PyObject* args)
{
    int size;
    PyObject* chunk_obj;
    PyObject* kernel_obj;
    if (!PyArg_ParseTuple(args, "OOi", &chunk_obj, &kernel_obj, &size)) {
        return NULL;
    }
    PyObject* chunk_array = PyArray_FROM_OTF(chunk_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_array = PyArray_FROM_OTF(chunk_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);

    if (chunk_array == NULL || kernel_array == NULL) {
        Py_XDECREF(chunk_array);
        Py_XDECREF(kernel_array);
        return NULL;
    }

    uint8_t* chunk = (uint8_t*)PyArray_DATA((PyArrayObject*)chunk_array);
    uint8_t* kernel = (uint8_t*)PyArray_DATA((PyArrayObject*)kernel_array);
    
    uint8_t res = convolve_gaussian(chunk, kernel, size);

    Py_DECREF(chunk_array);
    Py_DECREF(kernel_array);

    PyObject* out = Py_BuildValue("B", res);
    return out;
}
