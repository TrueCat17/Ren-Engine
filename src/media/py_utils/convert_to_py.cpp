#include "convert_to_py.h"


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
	return PyUnicode_FromStringAndSize(obj.c_str(), Py_ssize_t(obj.size()));
}



//if long == int, x86, 32-bit
// then long is independent type (is not synonym for int32_t)
//else (64-bit)
// do nothing, because long is int64_t
#if ((LONG_MAX) == (INT_MAX))
template<>
PyObject* convertToPy<>(long obj) {
	return PyLong_FromLong(obj);
}
template<>
PyObject* convertToPy<>(unsigned long obj) {
	return PyLong_FromUnsignedLong(obj);
}
#endif
