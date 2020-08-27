#ifndef CONVERT_FROM_PY_H
#define CONVERT_FROM_PY_H

#include <inttypes.h>
#include <string>

#include <Python.h>


struct PyConvertError {};


template<typename T>
T convertFromPy(PyObject *obj, const char *funcName, size_t argIndex);


template<>
PyObject* convertFromPy<PyObject*>(PyObject *obj, const char*, size_t);


template<>
bool convertFromPy<bool>(PyObject *obj, const char *funcName, size_t argIndex);

template<>
  int8_t convertFromPy<  int8_t>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
 uint8_t convertFromPy< uint8_t>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
 int16_t convertFromPy< int16_t>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
uint16_t convertFromPy<uint16_t>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
 int32_t convertFromPy< int32_t>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
uint32_t convertFromPy<uint32_t>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
 int64_t convertFromPy< int64_t>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
uint64_t convertFromPy<uint64_t>(PyObject *obj, const char *funcName, size_t argIndex);

//if long == int32_t, x86, 32-bit
#if ((LONG_MAX) == (INT_MAX))
template<>
inline
signed long int convertFromPy(PyObject* obj, const char *funcName, size_t argIndex) {
	return convertFromPy<int32_t>(obj, funcName, argIndex);
}
template<>
inline
unsigned long int convertFromPy(PyObject* obj, const char *funcName, size_t argIndex) {
	return convertFromPy<uint32_t>(obj, funcName, argIndex);
}
#endif


template<>
 float convertFromPy<float>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
double convertFromPy<double>(PyObject *obj, const char *funcName, size_t argIndex);


template<>
const char* convertFromPy<const char*>(PyObject *obj, const char *funcName, size_t argIndex);
template<>
std::string convertFromPy<std::string>(PyObject *obj, const char *funcName, size_t argIndex);

#endif // CONVERT_FROM_PY_H
