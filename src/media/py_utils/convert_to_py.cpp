#include "convert_to_py.h"


PyObject* convertToPy(PyObject *obj) {
	return obj;
}

PyObject* convertToPy(bool obj) {
	if (obj) {
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}


#define MAKE_CONVERT_INT_TO_PY(type) \
PyObject* convertToPy(type obj) { \
	return PyInt_FromLong(long(obj)); \
}

MAKE_CONVERT_INT_TO_PY(  int8_t)
MAKE_CONVERT_INT_TO_PY( uint8_t)
MAKE_CONVERT_INT_TO_PY( int16_t)
MAKE_CONVERT_INT_TO_PY(uint16_t)
MAKE_CONVERT_INT_TO_PY( int32_t)

//long: int32_t or int64_t

//if long == int64_t
#if ((LONG_MAX) == (LONG_LONG_MAX))
MAKE_CONVERT_INT_TO_PY(uint32_t)
MAKE_CONVERT_INT_TO_PY( int64_t)
#endif

//if long == int32_t
#if ((LONG_MAX) == (INT_MAX))
PyObject* convertToPy(uint32_t obj) {
	if (obj <= uint32_t(INT_MAX))
		return PyInt_FromLong(long(obj));
	return PyLong_FromUnsignedLongLong(obj);
}
PyObject* convertToPy(int64_t obj) {
	if (obj >= int64_t(INT_MIN) && obj <= int64_t(INT_MAX))
		return PyInt_FromLong(long(obj));
	return PyLong_FromLongLong(obj);
}
#endif

PyObject* convertToPy(uint64_t obj) {
	if (obj <= uint64_t(LONG_MAX))
		return PyInt_FromLong(long(obj));
	return PyLong_FromUnsignedLongLong(obj);
}


PyObject* convertToPy(float obj) {
	return PyFloat_FromDouble(double(obj));
}
PyObject* convertToPy(double obj) {
	return PyFloat_FromDouble(obj);
}

PyObject* convertToPy(const char *obj) {
	return PyString_FromString(obj);
}
PyObject* convertToPy(const std::string &obj) {
	return PyString_FromString(obj.c_str());
}
