#ifndef PY_SET_GLOBALS_H
#define PY_SET_GLOBALS_H

#include <Python.h>

class PySetGlobals {
public:
	static void set(PyObject *global, PyObject *builtinDict, PyObject *builtinStr);
};

#endif // PY_SET_GLOBALS_H
