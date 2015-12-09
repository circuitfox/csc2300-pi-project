#include <Python.h>
#include <stdio.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include "cutil.h"

static char cutil_docstring[] = "Module for utility functions written in C.";

static char rgb_to_l_docstring[] = "Convert RGB values into their Luminance (or greyscale) value.";
static char convert_to_greyscale_docstring[] = "Convert a buffer of RGB data to greyscale";

static PyObject* cutil_rgb_to_l(PyObject* self, PyObject* args);
static PyObject* cutil_convert_to_greyscale(PyObject* self, PyObject* args);

static PyMethodDef cutil_methods[] = {
    {"rgb_to_l", cutil_rgb_to_l, METH_VARARGS, rgb_to_l_docstring},
    {"convert_to_greyscale", cutil_convert_to_greyscale, METH_VARARGS, convert_to_greyscale_docstring},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef cutil_module = {
       PyModuleDef_HEAD_INIT,
       "pi_rtvp.cutil",  // name of module
       cutil_docstring,  // module documentation, may be NULL
       -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
       cutil_methods
};

PyMODINIT_FUNC PyInit_cutil(void)
{
    PyObject* m = PyModule_Create(&cutil_module);
    if (m == NULL)
        return NULL;
    import_array();
    return m;
}

// pi_rtvp.cutil.rgb_to_l(r, g, b)
static PyObject* cutil_rgb_to_l(PyObject* self, PyObject* args)
{
    double r, g, b;
    if (!PyArg_ParseTuple(args, "ddd", &r, &g, &b)) {
        return NULL;
    }
    unsigned char l = rgb_to_l(r, g, b);
    PyObject* ret = Py_BuildValue("B", l);
    return ret;
}

// pi_rtvp.cutil.convert_to_greyscale(data, height, width, planes)
static PyObject* cutil_convert_to_greyscale(PyObject* self, PyObject* args)
{
    int height, width, planes;
    PyObject* data_obj;
    if (!PyArg_ParseTuple(args, "Oiii", &data_obj, &height, &width, &planes)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    long len = height * width;
    uint8_t* grey = convert_to_greyscale(data, height, width, planes);
    
    PyObject* grey_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, grey);
    if (grey_array == NULL) {
        Py_DECREF(data_array);
        Py_XDECREF(grey_array);
        return NULL;
    }
    Py_DECREF(data_array);
    
    PyObject* ret = Py_BuildValue("O", grey_array);
    return ret;
}
