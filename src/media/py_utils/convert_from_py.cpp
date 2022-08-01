#include "convert_from_py.h"

#include <limits>

[[noreturn]]
void pySetErrorType(const char *funcName, size_t argIndex, PyObject *obj, const char *expectedType) {
	std::string err = std::string() +
		funcName + "(), argument #" + std::to_string(argIndex + 1) + ": "
		"expected " + expectedType + ", "
	    "got " + obj->ob_type->tp_name;
	PyErr_SetString(PyExc_TypeError, err.c_str());
	throw PyConvertError();
}

[[noreturn]]
void pySetErrorOverflow(const char *funcName, size_t argIndex, const char *msg, const char *type) {
	std::string err = std::string() +
		funcName + "(), argument #" + std::to_string(argIndex + 1) + ": " + msg + type;
	PyErr_SetString(PyExc_OverflowError, err.c_str());
	throw PyConvertError();
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
}


#define MAKE_CONVERT_FROM_PY_TO_INT(type, expectedType) \
template<> \
type convertFromPy<type>(PyObject *obj, const char *funcName, size_t argIndex) { \
	if (PyInt_CheckExact(obj)) { \
		long tmp = PyInt_AS_LONG(obj); \
		if constexpr (sizeof(type) < sizeof(long) || !std::numeric_limits<type>::is_signed) { \
			if (tmp < long(std::numeric_limits<type>::min())) { \
				pySetErrorOverflow(funcName, argIndex, "int too small to convert to ", #type); \
			} \
		} \
		if constexpr (sizeof(type) < sizeof(long)) { \
			if (tmp > long(std::numeric_limits<type>::max())) { \
				pySetErrorOverflow(funcName, argIndex, "int too large to convert to ", #type); \
			} \
		} \
		return type(tmp); \
	} \
	\
	if (PyFloat_CheckExact(obj)) { \
		double tmp = PyFloat_AS_DOUBLE(obj); \
		if constexpr (!(std::is_same<type, double>::value || std::is_same<type, float>::value)) { \
			if (tmp < double(std::numeric_limits<double>::min())) { \
				pySetErrorOverflow(funcName, argIndex, "float too small to convert to ", #type); \
			} \
			if (tmp > double(std::numeric_limits<type>::max())) { \
				pySetErrorOverflow(funcName, argIndex, "float too large to convert to ", #type); \
			} \
		} \
		return type(tmp); \
	} \
	\
	pySetErrorType(funcName, argIndex, obj, expectedType); \
}

MAKE_CONVERT_FROM_PY_TO_INT(  int8_t, "int")
MAKE_CONVERT_FROM_PY_TO_INT( uint8_t, "int")
MAKE_CONVERT_FROM_PY_TO_INT( int16_t, "int")
MAKE_CONVERT_FROM_PY_TO_INT(uint16_t, "int")
MAKE_CONVERT_FROM_PY_TO_INT( int32_t, "int")
MAKE_CONVERT_FROM_PY_TO_INT(uint32_t, "int")
MAKE_CONVERT_FROM_PY_TO_INT( int64_t, "int")
MAKE_CONVERT_FROM_PY_TO_INT(uint64_t, "int")

MAKE_CONVERT_FROM_PY_TO_INT( float, "float")
MAKE_CONVERT_FROM_PY_TO_INT(double, "float")


template<>
const char* convertFromPy<const char*>(PyObject *obj, const char *funcName, size_t argIndex) {
	if (PyString_CheckExact(obj)) {
		return PyString_AS_STRING(obj);
	}
	pySetErrorType(funcName, argIndex, obj, "str");
}
template<>
std::string convertFromPy<std::string>(PyObject *obj, const char *funcName, size_t argIndex) {
	if (PyString_CheckExact(obj)) {
		return PyString_AS_STRING(obj);
	}
	pySetErrorType(funcName, argIndex, obj, "str");
}
