#ifndef CONVERT_TO_PY_H
#define CONVERT_TO_PY_H

#include <Python.h>

template<typename T>
PyObject* convertToPy(T obj);


#endif // CONVERT_TO_PY_H
