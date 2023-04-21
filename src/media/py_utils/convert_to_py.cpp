#include "convert_to_py.h"

#include <string>

template<>
PyObject* convertToPy<>(PyObject *obj) {
	return obj;
}

template<>
PyObject* convertToPy<>(bool obj) {
	PyObject *res = obj ? Py_True : Py_False;
	Py_INCREF(res);
	return res;
}


#define MAKE_CONVERT_INT_TO_PY(type) \
template<> \
PyObject* convertToPy<>(type obj) { \
	return PyLong_FromLongLong((long long)(obj)); \
}
MAKE_CONVERT_INT_TO_PY( int8_t)
MAKE_CONVERT_INT_TO_PY(int16_t)
MAKE_CONVERT_INT_TO_PY(int32_t)
MAKE_CONVERT_INT_TO_PY(int64_t)


#define MAKE_CONVERT_UINT_TO_PY(type) \
template<> \
PyObject* convertToPy<>(type obj) { \
	return PyLong_FromUnsignedLongLong((unsigned long long)(obj)); \
}
MAKE_CONVERT_UINT_TO_PY( uint8_t)
MAKE_CONVERT_UINT_TO_PY(uint16_t)
MAKE_CONVERT_UINT_TO_PY(uint32_t)
MAKE_CONVERT_UINT_TO_PY(uint64_t)


template<>
PyObject* convertToPy<>(float obj) {
	return PyFloat_FromDouble(double(obj));
}
template<>
PyObject* convertToPy<>(double obj) {
	return PyFloat_FromDouble(obj);
}

template<>
PyObject* convertToPy<>(const char *obj) {
	return PyUnicode_FromString(obj);
}
template<>
PyObject* convertToPy<>(const std::string &obj) {
	return PyUnicode_FromStringAndSize(obj.c_str(), long(obj.size()));
}
