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

//if long == int32_t, x86, 32-bit
#if ((LONG_MAX) == (INT_MAX))
inline
PyObject* convertToPy(signed long int obj) {
	return convertToPy(static_cast<signed int>(obj));
}
inline
PyObject* convertToPy(unsigned long int obj) {
	return convertToPy(static_cast<unsigned int>(obj));
}
#else
//if long != int32_t, x86-64, 64-bit
inline
PyObject* convertToPy(signed long int obj) {
	return convertToPy(static_cast<signed long long>(obj));
}
inline
PyObject* convertToPy(unsigned long int obj) {
	return convertToPy(static_cast<unsigned long long>(obj));
}
#endif

PyObject* convertToPy( float obj);
PyObject* convertToPy(double obj);

PyObject* convertToPy(const char *obj);
PyObject* convertToPy(const std::string &obj);


#endif // CONVERT_TO_PY_H
