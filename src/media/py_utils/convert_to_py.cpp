#include "convert_to_py.h"


PyObject* convertToPy(PyObject *obj) {
	return obj;
}

PyObject* convertToPy(bool obj) {
	PyObject *res = obj ? Py_True : Py_False;
	Py_INCREF(res);
	return res;
}


#define MAKE_CONVERT_INT_TO_PY(type) \
PyObject* convertToPy(type obj) { \
	return PyInt_FromLong(long(obj)); \
}

MAKE_CONVERT_INT_TO_PY(  signed char)
MAKE_CONVERT_INT_TO_PY(unsigned char)
MAKE_CONVERT_INT_TO_PY(  signed short)
MAKE_CONVERT_INT_TO_PY(unsigned short)
MAKE_CONVERT_INT_TO_PY(  signed int)

//long: int32_t or int64_t

//if long == int64_t
#if ((LONG_MAX) == (LONG_LONG_MAX))
MAKE_CONVERT_INT_TO_PY(unsigned int)
MAKE_CONVERT_INT_TO_PY(  signed long long)
#endif

//if long == int32_t
#if ((LONG_MAX) == (INT_MAX))
PyObject* convertToPy(unsigned int obj) {
	if (obj <= uint32_t(INT_MAX))
		return PyInt_FromLong(long(obj));
	return PyLong_FromUnsignedLongLong(obj);
}
PyObject* convertToPy(signed long long obj) {
	if (obj >= int64_t(INT_MIN) && obj <= int64_t(INT_MAX))
		return PyInt_FromLong(long(obj));
	return PyLong_FromLongLong(obj);
}
#endif

PyObject* convertToPy(unsigned long long obj) {
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
