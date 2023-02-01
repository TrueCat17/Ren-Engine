#include "absolute.h"

#include <string>

#include "utils/math.h"

extern "C" {

PyObject* PyAbsolute_FromDouble(double val) {
	PyObject *res = PyObject_New(PyObject, &PyAbsolute_Type);
	PyFloat_AS_DOUBLE(res) = val;
	return res;
}


static PyObject *
absolute_new(PyTypeObject *, PyObject *args, PyObject *kwds) {
	double val = 0;
	
	if (kwds && PyDict_Size(kwds)) {
		PyErr_SetString(PyExc_RuntimeError, "absolute() not expected named arguments");
		return nullptr;
	}
	
	long countArgs = PySequence_Fast_GET_SIZE(args);
	if (countArgs) {
		if (countArgs > 1) {
			PyErr_SetString(PyExc_RuntimeError, "absolute() expected 1 optional argument");
			return nullptr;
		}
		PyObject *x = PySequence_Fast_GET_ITEM(args, 0);
		
		if (PyFloat_CheckExact(x) || PyAbsolute_CheckExact(x)) {
			val = PyFloat_AS_DOUBLE(x);
		}else
		if (PyInt_CheckExact(x)) {
			val = double(PyInt_AS_LONG(x));
		}else
		if (PyLong_CheckExact(x)) {
			val = PyLong_AsDouble(x);
		}else {
			std::string type = x->ob_type->tp_name;
			std::string desc = "absolute() argument must be float, int or long, got " + type;
			PyErr_SetString(PyExc_TypeError, desc.c_str());
			return nullptr;
		}
	}
	
	return PyAbsolute_FromDouble(val);
}

static void absolute_dealloc(PyObject *obj) {
	PyObject_Free(obj);
}


static PyObject* reduce(PyObject *self, PyObject *) {
	double value = PyFloat_AS_DOUBLE(self);
	PyObject *pyFloat = PyFloat_FromDouble(value);

	PyObject *args = PyTuple_New(1);
	PyTuple_SET_ITEM(args, 0, pyFloat);

	PyObject *res = PyTuple_New(2);
	PyObject *type = reinterpret_cast<PyObject*>(&PyAbsolute_Type);
	Py_INCREF(type);
	PyTuple_SET_ITEM(res, 0, type);
	PyTuple_SET_ITEM(res, 1, args);
	return res;
}
static long hash(PyObject *self) {
	double value = PyFloat_AS_DOUBLE(self);
	return _Py_HashDouble(value);
}

static PyObject* toStr(PyObject *self) {
	double value = PyFloat_AS_DOUBLE(self);
	std::string str = "absolute(" + std::to_string(value) + ")";

	PyObject *res = PyString_FromStringAndSize(str.c_str(), long(str.size()));
	return res;
}

static PyMethodDef methods[] = {
    {"__reduce__", reduce, METH_NOARGS, "helper for pickle"},
    {nullptr, nullptr, 0, nullptr}
};

#define CONVERT_TO_DOUBLE(obj, dbl) \
if (PyAbsolute_CheckExact(obj) || PyFloat_CheckExact(obj)) { \
	dbl = PyFloat_AS_DOUBLE(obj); \
}else if (PyInt_CheckExact(obj)) { \
	dbl = double(PyInt_AS_LONG(obj)); \
}else if (PyLong_CheckExact(obj)) { \
	dbl = PyLong_AsDouble(obj); \
}else { \
	std::string desc = "absolute.__" + std::string(__func__) + "__ expected absolute, float, int or long types, got " + obj->ob_type->tp_name; \
	PyErr_SetString(PyExc_TypeError, desc.c_str()); \
	return nullptr; \
}

#define PREPARE_DOUBLES \
double a, b; \
CONVERT_TO_DOUBLE(pyA, a) \
CONVERT_TO_DOUBLE(pyB, b)

static PyObject* add(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	return PyAbsolute_FromDouble(a + b);
}
static PyObject* sub(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	return PyAbsolute_FromDouble(a - b);
}
static PyObject* mul(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	return PyAbsolute_FromDouble(a * b);
}
static PyObject* getDiv(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	if (Math::doublesAreEq(b, 0)) {
		PyErr_SetString(PyExc_ZeroDivisionError, "absolute division by zero");
		return nullptr;
	}
	return PyAbsolute_FromDouble(a / b);
}
static PyObject* rem(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	if (Math::doublesAreEq(b, 0)) {
		PyErr_SetString(PyExc_ZeroDivisionError, "absolute module");
		return nullptr;
	}

	double mod = std::fmod(a, b);
	if ((b < 0) != (mod < 0)) {
		mod += b;
	}
	return PyAbsolute_FromDouble(mod);
}


static PyObject* neg(PyObject *pyA) {
	return PyAbsolute_FromDouble(-PyFloat_AS_DOUBLE(pyA));
}
static PyObject* pos(PyObject *pyA) {
	return PyAbsolute_FromDouble(PyFloat_AS_DOUBLE(pyA));
}
static PyObject* getAbs(PyObject *pyA) {
	return PyAbsolute_FromDouble(std::abs(PyFloat_AS_DOUBLE(pyA)));
}
static int nonzero(PyObject *pyA) {
	return !Math::doublesAreEq(PyFloat_AS_DOUBLE(pyA), 0);
}


static PyObject* toInt(PyObject *pyA) {
	return PyInt_FromLong(long(PyFloat_AS_DOUBLE(pyA)));
}
static PyObject* toLong(PyObject *pyA) {
	return PyLong_FromLong(long(PyFloat_AS_DOUBLE(pyA)));
}
static PyObject* toFloat(PyObject *pyA) {
	return PyFloat_FromDouble(PyFloat_AS_DOUBLE(pyA));
}


static PyNumberMethods asNumber = {
    add,      /*nb_add*/
    sub,      /*nb_subtract*/
    mul,      /*nb_multiply*/
    getDiv,   /*nb_divide*/
    rem,      /*nb_remainder*/
    nullptr,  /*nb_divmod*/
    nullptr,  /*nb_power*/
    neg,      /*nb_negative*/
    pos,      /*nb_positive*/
    getAbs,   /*nb_absolute*/
    nonzero,  /*nb_nonzero*/
    nullptr,  /*nb_invert*/
    nullptr,  /*nb_lshift*/
    nullptr,  /*nb_rshift*/
    nullptr,  /*nb_and*/
    nullptr,  /*nb_xor*/
    nullptr,  /*nb_or*/
    nullptr,  /*nb_coerce*/
    toInt,    /*nb_int*/
    toLong,   /*nb_long*/
    toFloat,  /*nb_float*/
    nullptr,  /* nb_oct */
    nullptr,  /* nb_hex */
    nullptr,  /* nb_inplace_add */
    nullptr,  /* nb_inplace_subtract */
    nullptr,  /* nb_inplace_multiply */
    nullptr,  /* nb_inplace_divide */
    nullptr,  /* nb_inplace_remainder */
    nullptr,  /* nb_inplace_power */
    nullptr,  /* nb_inplace_lshift */
    nullptr,  /* nb_inplace_rshift */
    nullptr,  /* nb_inplace_and */
    nullptr,  /* nb_inplace_xor */
    nullptr,  /* nb_inplace_or */
    nullptr,  /* nb_floor_divide */
    getDiv,   /* nb_true_divide */
    nullptr,  /* nb_inplace_floor_divide */
    nullptr,  /* nb_inplace_true_divide */
    nullptr,  /* nb_index */
};


static PyObject* richcompare(PyObject *pyA, PyObject *pyB, int op) {
	bool res = false;
	if (pyB == Py_None) {
		     if (op == Py_LT) res = false;
		else if (op == Py_LE) res = false;
		else if (op == Py_EQ) res = false;
		else if (op == Py_NE) res = true;
		else if (op == Py_GT) res = true;
		else if (op == Py_GE) res = true;
	}else {
		PREPARE_DOUBLES
		     if (op == Py_LT) res = a < b;
		else if (op == Py_LE) res = a <= b;
		else if (op == Py_EQ) res = Math::doublesAreEq(a, b);
		else if (op == Py_NE) res = !Math::doublesAreEq(a, b);
		else if (op == Py_GT) res = a > b;
		else if (op == Py_GE) res = a >= b;
	}

	PyObject *pyRes = res ? Py_True : Py_False;
	Py_INCREF(pyRes);
	return pyRes;
}


PyTypeObject PyAbsolute_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"absolute",
    sizeof(PyAbsoluteObject),
	0,
	absolute_dealloc,                           /* tp_dealloc */
    nullptr,                                    /* tp_print */
    nullptr,                                    /* tp_getattr */
    nullptr,                                    /* tp_setattr */
    nullptr,                                    /* tp_compare */
    toStr,                                      /* tp_repr */
    &asNumber,                                  /* tp_as_number */
    nullptr,                                    /* tp_as_sequence */
    nullptr,                                    /* tp_as_mapping */
    hash,                                       /* tp_hash */
    nullptr,                                    /* tp_call */
    toStr,                                      /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    nullptr,                                    /* tp_setattro */
    nullptr,                                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES, /* tp_flags */
    "Absolute coordinate in float.",            /* tp_doc */
    nullptr,                                    /* tp_traverse */
    nullptr,                                    /* tp_clear */
    richcompare,                                /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    nullptr,                                    /* tp_iter */
    nullptr,                                    /* tp_iternext */
    methods,                                    /* tp_methods */
    nullptr,                                    /* tp_members */
    nullptr,                                    /* tp_getset */
    nullptr,                                    /* tp_base */
    nullptr,                                    /* tp_dict */
    nullptr,                                    /* tp_descr_get */
    nullptr,                                    /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    nullptr,                                    /* tp_init */
    nullptr,                                    /* tp_alloc */
	absolute_new,                               /* tp_new */
    nullptr,                                    /* tp_free */
    nullptr,                                    /* tp_is_gc */
    nullptr,                                    /* tp_bases */
    nullptr,                                    /* tp_mro */
    nullptr,                                    /* tp_cache */
    nullptr,                                    /* tp_subclasses */
    nullptr,                                    /* tp_weaklist */
    nullptr,                                    /* tp_del */
    0,                                          /* tp_version_tag */
};


}
