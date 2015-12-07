#include <Python.h>

#include "cutil.h"

static char cutil_docstring[] = "Module for utility functions written in C.";

static char rgb_to_l_docstring[] = "Convert RGB values into their Luminance (or greyscale) value.";

static PyObject* cutil_rgb_to_l(PyObject* self, PyObject* args);

static PyMethodDef cutil_methods[] = {
    {"rgb_to_l", cutil_rgb_to_l, METH_VARARGS, rgb_to_l_docstring},
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
    return m;
}

// pi_rtvp.cutil.rgb_to_l(r, g, b)
static PyObject* cutil_rgb_to_l(PyObject* self, PyObject* args)
{
    double r, g, b;
    if (!PyArg_ParseTuple(args, "ddd", &r, &g, &b)) {
        return NULL;
    }
    unsigned char l = rtvp_rgb_to_l(r, g, b);
    PyObject* ret = Py_BuildValue("B", l);
    return ret;
}
