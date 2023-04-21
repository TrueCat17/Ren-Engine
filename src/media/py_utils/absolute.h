#include <Python.h>

extern "C" {


typedef PyFloatObject PyAbsoluteObject;

extern PyTypeObject PyAbsolute_Type;

#define PyAbsolute_CheckExact(x) (Py_TYPE(x) == &PyAbsolute_Type)

PyObject* PyAbsolute_FromDouble(double);


}
