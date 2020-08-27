#include <Python.h>

extern "C" {


typedef PyFloatObject PyAbsoluteObject;

PyAPI_DATA(PyTypeObject) PyAbsolute_Type;

#define PyAbsolute_CheckExact(x) (Py_TYPE(x) == &PyAbsolute_Type)

PyAPI_FUNC(PyObject *) PyAbsolute_FromDouble(double);

}
