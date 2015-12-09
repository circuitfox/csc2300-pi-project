#include <stdint.h>

#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include "matrix.h"

static char matrix_docstring[] = "Module for fast kernel convolution";
static char is_corner_docstring[] = "Determine if the group of points is at a matrix corner";
static char get_matrix_at_docstring[] = "Return matrix of size centered around (x, y)";
static char center_matrix_docstring[] = "Return matrix of size centered around (x, y)";
static char corner_matrix_docstring[] = "Return matrix of size centered around (x, y) corner";
static char edge_matrix_docstring[] = "Return matrix of size centered around (x, y) edge";

static PyObject* matrix_convolve(PyObject* self, PyObject* args);
static PyObject* matrix_convolve_id(PyObject* self, PyObject* args);
static PyObject* matrix_convolve_sobel(PyObject* self, PyObject* args);
static PyObject* matrix_convolve_gaussian(PyObject* self, PyObject* args);
static PyObject* matrix_convolve_image(PyObject* self, PyObject* args);
static PyObject* matrix_convolve_image_id(PyObject* self, PyObject* args);
static PyObject* matrix_convolve_image_gaussian(PyObject* self, PyObject* args);
static PyObject* matrix_convolve_image_sobel(PyObject* self, PyObject* args);

static PyObject* matrix_is_corner(PyObject* self, PyObject* args);
static PyObject* matrix_get_matrix_at(PyObject* self, PyObject* args);
static PyObject* matrix_center_matrix(PyObject* self, PyObject* args);
static PyObject* matrix_corner_matrix(PyObject* self, PyObject* args);
static PyObject* matrix_edge_matrix(PyObject* self, PyObject* args);

static PyMethodDef convolve_methods[] = {
    {"convolve", matrix_convolve, METH_VARARGS, NULL},
    {"convolve_id", matrix_convolve_id, METH_VARARGS, NULL},
    {"convolve_sobel", matrix_convolve_sobel, METH_VARARGS, NULL},
    {"convolve_gaussian", matrix_convolve_gaussian, METH_VARARGS, NULL},
    {"convolve_image", matrix_convolve_image, METH_VARARGS, NULL},
    {"convolve_image_id", matrix_convolve_image_id, METH_VARARGS, NULL},
    {"convolve_image_gaussian", matrix_convolve_image_gaussian, METH_VARARGS, NULL},
    {"convolve_image_sobel", matrix_convolve_image_sobel, METH_VARARGS, NULL},
    {"is_corner", matrix_is_corner, METH_VARARGS, is_corner_docstring},
    {"get_matrix_at", matrix_get_matrix_at, METH_VARARGS, get_matrix_at_docstring},
    {"center_matrix", matrix_center_matrix, METH_VARARGS, center_matrix_docstring},
    {"corner_matrix", matrix_corner_matrix, METH_VARARGS, corner_matrix_docstring},
    {"edge_matrix", matrix_edge_matrix, METH_VARARGS, edge_matrix_docstring},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef matrix_module = {
       PyModuleDef_HEAD_INIT,
       "pi_rtvp.matrix",  // name of module
       matrix_docstring,  // module documentation, may be NULL
       -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
       convolve_methods
};

PyMODINIT_FUNC PyInit_matrix(void)
{
    PyObject* m = PyModule_Create(&matrix_module);
    if (m == NULL)
        return NULL;
    import_array();
    return m;
}

// pi_rtvp.matrix.convolve(data, kernel, size)
static PyObject* matrix_convolve(PyObject* self, PyObject* args)
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

// pi_rtvp.matrix.convolve_id(data, size)
static PyObject* matrix_convolve_id(PyObject* self, PyObject* args)
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

// pi_rtvp.matrix.convolve(data, kernel_x, kernel_y, size)
static PyObject* matrix_convolve_sobel(PyObject* self, PyObject* args)
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

// pi_rtvp.matrix.convolve_gaussian(data, kernel, size)
static PyObject* matrix_convolve_gaussian(PyObject* self, PyObject* args)
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
    uint16_t* kernel = (uint16_t*)PyArray_DATA((PyArrayObject*)kernel_array);
    
    uint8_t res = convolve_gaussian(chunk, kernel, size);

    Py_DECREF(chunk_array);
    Py_DECREF(kernel_array);

    PyObject* out = Py_BuildValue("B", res);
    return out;
}

// pi_rtvp.matrix.convolve_image(data, kernel, height, width, size)
static PyObject* matrix_convolve_image(PyObject* self, PyObject* args)
{
    int height, width, size;
    PyObject* data_obj;
    PyObject* kernel_obj;
    if (!PyArg_ParseTuple(args, "OOiii", &data_obj, &kernel_obj, &height, &width, &size)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_array = PyArray_FROM_OTF(kernel_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);

    if (data_array == NULL || kernel_array == NULL) {
        Py_XDECREF(data_array);
        Py_XDECREF(kernel_array);
        return NULL;
    }

    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    uint8_t* kernel = (uint8_t*)PyArray_DATA((PyArrayObject*)kernel_array);
    long len = width*height;
    
    uint8_t* res = convolve_image(data, kernel, height, width, size);

    PyObject* image_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, res);
    if (image_array == NULL) {
        Py_DECREF(data_array);
        Py_DECREF(kernel_array);
        Py_XDECREF(image_array);
        return NULL;
    }
    Py_DECREF(data_array);
    Py_DECREF(kernel_array);

    PyObject* out = Py_BuildValue("O", image_array);
    return out;
}

// pi_rtvp.matrix.convolve_image_id(data, height, width, size)
static PyObject* matrix_convolve_image_id(PyObject* self, PyObject* args)
{
    int height, width, size;
    PyObject* data_obj;
    if (!PyArg_ParseTuple(args, "Oiii", &data_obj, &height, &width, &size)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);

    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }

    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    long len = width*height;
    
    uint8_t* res = convolve_image_id(data, height, width, size);

    PyObject* image_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, res);
    if (image_array == NULL) {
        Py_DECREF(data_array);
        Py_XDECREF(image_array);
        return NULL;
    }
    Py_DECREF(data_array);

    PyObject* out = Py_BuildValue("O", image_array);
    return out;
}

// pi_rtvp.matrix.convolve_image_gaussian(data, kernel, height, width, size)
static PyObject* matrix_convolve_image_gaussian(PyObject* self, PyObject* args)
{
    int height, width, size;
    PyObject* data_obj;
    PyObject* kernel_obj;
    if (!PyArg_ParseTuple(args, "OOiii", &data_obj, &kernel_obj, &height, &width, &size)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_array = PyArray_FROM_OTF(kernel_obj, NPY_UINT16, NPY_ARRAY_IN_ARRAY);

    if (data_array == NULL || kernel_array == NULL) {
        Py_XDECREF(data_array);
        Py_XDECREF(kernel_array);
        return NULL;
    }

    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    uint16_t* kernel = (uint16_t*)PyArray_DATA((PyArrayObject*)kernel_array);
    long len = width*height;
    
    uint8_t* res = convolve_image_gaussian(data, kernel, height, width, size);

    PyObject* image_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, res);
    if (image_array == NULL) {
        Py_DECREF(data_array);
        Py_DECREF(kernel_array);
        Py_XDECREF(image_array);
        return NULL;
    }
    Py_DECREF(data_array);
    Py_DECREF(kernel_array);

    PyObject* out = Py_BuildValue("O", image_array);
    return out;
}

// pi_rtvp.matrix.convolve_image_sobel(data, kernel_x, kernel_y, height, width, size)
static PyObject* matrix_convolve_image_sobel(PyObject* self, PyObject* args)
{
    int height, width, size;
    PyObject* data_obj;
    PyObject* kernel_x_obj;
    PyObject* kernel_y_obj;
    if (!PyArg_ParseTuple(args, "OOOiii", &data_obj, &kernel_x_obj, &kernel_y_obj,
                                         &height, &width, &size)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_x_array = PyArray_FROM_OTF(kernel_x_obj, NPY_INT8, NPY_ARRAY_IN_ARRAY);
    PyObject* kernel_y_array = PyArray_FROM_OTF(kernel_y_obj, NPY_INT8, NPY_ARRAY_IN_ARRAY);

    if (data_array == NULL || kernel_x_array == NULL || kernel_y_array == NULL) {
        Py_XDECREF(data_array);
        Py_XDECREF(kernel_x_array);
        Py_XDECREF(kernel_y_array);
        return NULL;
    }

    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    int8_t* kernel_x = (int8_t*)PyArray_DATA((PyArrayObject*)kernel_x_array);
    int8_t* kernel_y = (int8_t*)PyArray_DATA((PyArrayObject*)kernel_y_array);
    long len = width*height;
    
    uint8_t* res = convolve_image_sobel(data, kernel_x, kernel_y, height, width, size);

    PyObject* image_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, res);
    if (image_array == NULL) {
        Py_DECREF(data_array);
        Py_DECREF(kernel_x_array);
        Py_DECREF(kernel_y_array);
        Py_XDECREF(image_array);
        return NULL;
    }
    Py_DECREF(data_array);
    Py_DECREF(kernel_x_array);
    Py_DECREF(kernel_y_array);

    PyObject* out = Py_BuildValue("O", image_array);
    return out;
}

// pi_rtvp.matrix.is_corner(y, x, height, width, median)
static PyObject* matrix_is_corner(PyObject* self, PyObject* args)
{
    int y, x, height, width, median;
    if (!PyArg_ParseTuple(args, "iiiii", &y, &x, &height, &width, &median)) {
        return NULL;
    }
    int bound = is_corner(y, x, height, width, median);
    PyObject* ret = Py_BuildValue("N", PyBool_FromLong(bound));
    return ret;

}

// pi_rtvp.matrix.get_matrix_at(data, y, x, height, width, size)
static PyObject* matrix_get_matrix_at(PyObject* self, PyObject* args)
{
    int y, x, height, width, size;
    PyObject* data_obj;
    if (!PyArg_ParseTuple(args, "Oiiiii", &data_obj, &y, &x, &height, &width, &size)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    long len = size*size;

    uint8_t* matrix = get_matrix_at(data, y, x, height, width, size);

    PyObject* matrix_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, matrix);
    if (matrix_array == NULL) {
        Py_DECREF(data_array);
        Py_XDECREF(matrix_array);
        return NULL;
    }
    Py_DECREF(data_array);
    
    PyObject* ret = Py_BuildValue("O", matrix_array);
    return ret;
}

// pi_rtvp.matrix.center_matrix(data, y, x, width, size)
static PyObject* matrix_center_matrix(PyObject* self, PyObject* args)
{
    int y, x, width, size;
    PyObject* data_obj;
    if (!PyArg_ParseTuple(args, "Oiiii", &data_obj, &y, &x, &width, &size)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    long len = size*size;

    uint8_t* matrix = center_matrix(data, y, x, width, size);

    PyObject* matrix_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, matrix);
    if (matrix_array == NULL) {
        Py_DECREF(data_array);
        Py_XDECREF(matrix_array);
        return NULL;
    }
    Py_DECREF(data_array);
    
    PyObject* ret = Py_BuildValue("O", matrix_array);
    return ret;
}

// pi_rtvp.matrix.corner_matrix(data, y, x, width, size, type)
static PyObject* matrix_corner_matrix(PyObject* self, PyObject* args)
{
    int y, x, width, size, type;
    PyObject* data_obj;
    if (!PyArg_ParseTuple(args, "Oiiiii", &data_obj, &y, &x, &width, &size, &type)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    long len = size*size;

    uint8_t* matrix = corner_matrix(data, y, x, width, size, type);

    PyObject* matrix_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, matrix);
    if (matrix_array == NULL) {
        Py_DECREF(data_array);
        Py_XDECREF(matrix_array);
        return NULL;
    }
    Py_DECREF(data_array);
    
    PyObject* ret = Py_BuildValue("O", matrix_array);
    return ret;

}

// pi_rtvp.matrix.edge_matrix(data, y, x, width, size, type)
static PyObject* matrix_edge_matrix(PyObject* self, PyObject* args)
{
    int y, x, width, size, type;
    PyObject* data_obj;
    if (!PyArg_ParseTuple(args, "Oiiiii", &data_obj, &y, &x, &width, &size, &type)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    long len = size*size;

    uint8_t* matrix = edge_matrix(data, y, x, width, size, type);

    PyObject* matrix_array = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, matrix);
    if (matrix_array == NULL) {
        Py_DECREF(data_array);
        Py_XDECREF(matrix_array);
        return NULL;
    }
    Py_DECREF(data_array);
    
    PyObject* ret = Py_BuildValue("O", matrix_array);
    return ret;

}
