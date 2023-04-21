#ifndef CONVERT_FROM_PY_H
#define CONVERT_FROM_PY_H

#include <Python.h>


extern bool pyErrorFlag;

template<typename T>
T convertFromPy(PyObject *obj, const char *funcName, size_t argIndex);


#endif // CONVERT_FROM_PY_H
