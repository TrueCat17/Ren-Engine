#ifndef CONVERT_TO_PY_H
#define CONVERT_TO_PY_H

#include <inttypes.h>
#include <string>

#include <Python.h>

PyObject* convertToPy(PyObject *obj);

PyObject* convertToPy(    bool obj);


PyObject* convertToPy(  signed char obj);
PyObject* convertToPy(unsigned char obj);
PyObject* convertToPy(  signed short obj);
PyObject* convertToPy(unsigned short obj);
PyObject* convertToPy(  signed int obj);
PyObject* convertToPy(unsigned int obj);
PyObject* convertToPy(  signed long long obj);
PyObject* convertToPy(unsigned long long obj);

PyObject* convertToPy(signed long int obj);
PyObject* convertToPy(unsigned long int obj);


PyObject* convertToPy( float obj);
PyObject* convertToPy(double obj);

PyObject* convertToPy(const char *obj);
PyObject* convertToPy(const std::string &obj);


#endif // CONVERT_TO_PY_H
