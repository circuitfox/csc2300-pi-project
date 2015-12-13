#include <Python.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include "image.h"

static char image_docstring[] = "Python interface for libpng.";

static PyObject* image_read_png(PyObject* self, PyObject* args);
static PyObject* image_read_png_data(PyObject* self, PyObject* args);
static PyObject* image_write_png(PyObject* self, PyObject* args);
static PyObject* image_write_png_io(PyObject* self, PyObject* args);

static PyMethodDef image_methods[] = {
    {"read_png", image_read_png, METH_VARARGS, NULL},
    {"read_png_data", image_read_png_data, METH_VARARGS, NULL},
    {"write_png", image_write_png, METH_VARARGS, NULL},
    {"write_png_io", image_write_png_io, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef image_module = {
       PyModuleDef_HEAD_INIT,
       "pi_rtvp.image",  // name of module
       image_docstring,  // module documentation, may be NULL
       -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
       image_methods
};

PyMODINIT_FUNC PyInit_image(void)
{
    PyObject* m = PyModule_Create(&image_module);
    if (m == NULL)
        return NULL;
    import_array();
    return m;
}

// pi_rtvp.image.read_png(in)
static PyObject* image_read_png(PyObject* self, PyObject* args)
{
    char* in;
    struct png_data* png;
    if (!PyArg_ParseTuple(args, "s", &in)) {
        return NULL;
    }
    png = read_png(in);
    PyObject* info = PyDict_New();
    long len = png->width * png->height * png->planes;
    PyDict_SetItemString(info, "bit_depth", PyLong_FromLong(png->bit_depth));
    PyDict_SetItemString(info, "planes", PyLong_FromLong(png->planes));
    PyDict_SetItemString(info, "color_type", PyLong_FromLong(png->color_type));
    PyObject* png_array = PyArray_SimpleNewFromData(1, &len , NPY_UINT8, png->data);
    if (png_array == NULL) {
        Py_XDECREF(png_array);
        return NULL;
    }
    PyObject* ret = Py_BuildValue("iiOO", png->width, png->height, png_array, info);
    free(png);
    return ret;
}

// pi_rtvp.image.read_png_data(data)
static PyObject* image_read_png_data(PyObject* self, PyObject* args)
{
    PyObject* data_obj;
    struct png_data* png;
    if (!PyArg_ParseTuple(args, "O", &data_obj)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);

    png = read_png_buffer(data);
    PyObject* info = PyDict_New();
    long len = png->width * png->height * png->planes;
    PyDict_SetItemString(info, "bit_depth", PyLong_FromLong(png->bit_depth));
    PyDict_SetItemString(info, "planes", PyLong_FromLong(png->planes));
    PyDict_SetItemString(info, "color_type", PyLong_FromLong(png->color_type));

    PyObject* png_array = PyArray_SimpleNewFromData(1, &len , NPY_UINT8, png->data);
    if (png_array == NULL) {
        Py_DECREF(data_array);
        Py_XDECREF(png_array);
        return NULL;
    }
    Py_DECREF(data_array);
    PyObject* ret = Py_BuildValue("iiOO", png->width, png->height, png_array, info);
    free(png);
    return ret;
}

// pi_rtvp.image.write_png(out, width, height, data, info)
static PyObject* image_write_png(PyObject* self, PyObject* args)
{
    PyObject* data_obj;
    PyObject* info;
    uint32_t width, height;
    char* out;
    struct png_data png = {};
    if (!PyArg_ParseTuple(args, "siiOO", &out, &width, &height, &data_obj, &info)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    png.width = width;
    png.height = height;
    png.bit_depth = (int)PyLong_AsLong(PyDict_GetItemString(info, "bit_depth"));
    png.planes = (int)PyLong_AsLong(PyDict_GetItemString(info, "planes"));
    png.color_type = (int)PyLong_AsLong(PyDict_GetItemString(info, "color_type"));
    png.data = data;
    write_png(out, &png);
    Py_DECREF(data_array);
    Py_RETURN_NONE;
}

static PyObject* image_write_png_io(PyObject* self, PyObject* args)
{
    PyObject* data_obj;
    PyObject* info;
    PyObject* out;
    uint32_t width, height;
    struct png_data png = {};
    if (!PyArg_ParseTuple(args, "OiiOO", &out, &width, &height, &data_obj, &info)) {
        return NULL;
    }
    PyObject* data_array = PyArray_FROM_OTF(data_obj, NPY_UINT8, NPY_ARRAY_IN_ARRAY);
    if (data_array == NULL) {
        Py_XDECREF(data_array);
        return NULL;
    }
    uint8_t* data = (uint8_t*)PyArray_DATA((PyArrayObject*)data_array);
    png.width = width;
    png.height = height;
    png.bit_depth = (int)PyLong_AsLong(PyDict_GetItemString(info, "bit_depth"));
    png.planes = (int)PyLong_AsLong(PyDict_GetItemString(info, "planes"));
    png.color_type = (int)PyLong_AsLong(PyDict_GetItemString(info, "color_type"));
    png.data = data;
    struct png_buffer* out_buffer = write_png_buffer(&png);
    PyObject* result = PyObject_CallMethod(out, "write", "y#", out_buffer->buffer, out_buffer->position);
    if (!result) {
        Py_DECREF(data_array);
        free(out_buffer->buffer);
        free(out_buffer);
        return NULL;
    }
    Py_DECREF(data_array);
    free(out_buffer->buffer);
    free(out_buffer);
    Py_RETURN_NONE;
}
