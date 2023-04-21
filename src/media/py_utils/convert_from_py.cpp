#include "convert_from_py.h"

#include <limits>
#include <string>

bool pyErrorFlag = false;


static void pySetErrorType(const char *funcName, size_t argIndex, PyObject *obj, const char *expectedType) {
	if (pyErrorFlag) return;//dont overwrite prev error
	pyErrorFlag = true;

	PyErr_Format(PyExc_TypeError,
	    "%s(), argument #%zu: expected %s, got %s (%.50R)",
	    funcName, argIndex + 1, expectedType, obj->ob_type->tp_name, obj);
}

static void pySetErrorOverflow(const char *funcName, size_t argIndex, PyObject* obj, const char *msg) {
	if (pyErrorFlag) return;//dont overwrite prev error
	pyErrorFlag = true;

	PyErr_Format(PyExc_OverflowError,
	    "%s(), argument #%zu: %s (%.50R) %s",
	    funcName, argIndex + 1, PyLong_CheckExact(obj) ? "int" : "float", obj, msg);
}


template<>
PyObject* convertFromPy<PyObject*>(PyObject *obj, const char*, size_t) {
	return obj;
}


template<>
bool convertFromPy<bool>(PyObject *obj, const char *funcName, size_t argIndex) {
	if (PyBool_Check(obj)) {
		return obj == Py_True;
	}
	pySetErrorType(funcName, argIndex, obj, "bool");
	return false;
}


#define MAKE_CONVERT_FROM_PY_TO_INT(type) \
template<> \
type convertFromPy<type>(PyObject *obj, const char *funcName, size_t argIndex) { \
	bool isLong = PyLong_CheckExact(obj); \
	bool isFloat = !isLong && PyFloat_CheckExact(obj); \
	if (!isLong && !isFloat) { \
	    pySetErrorType(funcName, argIndex, obj, \
	        std::is_floating_point_v<type> ? "float" : "int"); \
	    return 0; \
	} \
	double tmp; \
	if (isLong) { \
	    tmp = PyLong_AsDouble(obj); \
	    if (PyErr_Occurred()) { \
	        pySetErrorOverflow(funcName, argIndex, obj, "too large to convert to "#type); \
	        return 0; \
	    } \
	}else { \
	    tmp = PyFloat_AS_DOUBLE(obj); \
	} \
	if constexpr (!std::is_floating_point_v<type>) { \
	    if (tmp < double(std::numeric_limits<type>::min())) { \
	        pySetErrorOverflow(funcName, argIndex, obj, "too small to convert to "#type); \
	        return 0; \
	    } \
	    if (tmp > double(std::numeric_limits<type>::max())) { \
	        pySetErrorOverflow(funcName, argIndex, obj, "too large to convert to "#type); \
	        return 0; \
	    } \
	} \
	return type(tmp); \
}

MAKE_CONVERT_FROM_PY_TO_INT(  int8_t)
MAKE_CONVERT_FROM_PY_TO_INT( uint8_t)
MAKE_CONVERT_FROM_PY_TO_INT( int16_t)
MAKE_CONVERT_FROM_PY_TO_INT(uint16_t)
MAKE_CONVERT_FROM_PY_TO_INT( int32_t)
MAKE_CONVERT_FROM_PY_TO_INT(uint32_t)
MAKE_CONVERT_FROM_PY_TO_INT( int64_t)
MAKE_CONVERT_FROM_PY_TO_INT(uint64_t)

MAKE_CONVERT_FROM_PY_TO_INT( float)
MAKE_CONVERT_FROM_PY_TO_INT(double)


#define MAKE_CONVERT_FROM_PY_TO_STR(type) \
template<> \
type convertFromPy<type>(PyObject *obj, const char *funcName, size_t argIndex) { \
	if (PyUnicode_CheckExact(obj)) { \
	    return PyUnicode_AsUTF8(obj); \
	} \
	pySetErrorType(funcName, argIndex, obj, "str"); \
	return ""; \
}

MAKE_CONVERT_FROM_PY_TO_STR(const char *)
MAKE_CONVERT_FROM_PY_TO_STR(std::string)
