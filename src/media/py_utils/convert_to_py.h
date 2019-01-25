#ifndef CONVERT_TO_PY_H
#define CONVERT_TO_PY_H

#include <inttypes.h>
#include <string>

#include <Python.h>

PyObject* convertToPy(PyObject *obj);

PyObject* convertToPy(    bool obj);

PyObject* convertToPy(  int8_t obj);
PyObject* convertToPy( uint8_t obj);
PyObject* convertToPy( int16_t obj);
PyObject* convertToPy(uint16_t obj);
PyObject* convertToPy( int32_t obj);
PyObject* convertToPy(uint32_t obj);
PyObject* convertToPy( int64_t obj);
PyObject* convertToPy(uint64_t obj);

PyObject* convertToPy( float obj);
PyObject* convertToPy(double obj);

PyObject* convertToPy(const char *obj);
PyObject* convertToPy(const std::string &obj);


#endif // CONVERT_TO_PY_H
