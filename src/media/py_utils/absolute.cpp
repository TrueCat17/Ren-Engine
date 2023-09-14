#include "absolute.h"

#include <string>

#include "utils/math.h"

extern "C" {

bool PyAbsolute_PreInit() {
	PyMethodDef *round = nullptr;
	PyMethodDef *methods = PyFloat_Type.tp_methods;
	while (methods->ml_name) {
		if (std::string(methods->ml_name) == "__round__") {
			round = methods;
			break;
		}
		++methods;
	}
	if (!round) return false;

	methods = PyAbsolute_Type.tp_methods;
	while (methods->ml_name) {
		if (std::string(methods->ml_name) == "__round__") {
			*methods = *round;
			return true;
		}
		++methods;
	}

	return false;
}

PyObject* PyAbsolute_FromDouble(double val) {
	PyObject *res = PyFloat_FromDouble(val);
	res->ob_type = &PyAbsolute_Type;
	return res;
}


static PyObject* absolute_new_imp(PyObject *arg, PyObject *kwargs) {
	if (kwargs && PyDict_Size(kwargs)) {
		PyErr_SetString(PyExc_TypeError, "absolute() takes no keyword arguments");
		return nullptr;
	}

	double val = 0;
	if (arg) {
		if (PyAbsolute_CheckExact(arg)) {
			return Py_NewRef(arg);
		}

		if (PyFloat_CheckExact(arg)) {
			val = PyFloat_AS_DOUBLE(arg);
		}else
		if (PyLong_CheckExact(arg)) {
			val = PyLong_AsDouble(arg);
			if (PyErr_Occurred()) {
				PyErr_SetString(PyExc_OverflowError, "int too large to convert to absolute");
				return nullptr;
			}
		}else {
			return PyErr_Format(PyExc_TypeError,
			                    "absolute() argument must be absolute, float or int, got %s", arg->ob_type->tp_name);
		}
	}
	return PyAbsolute_FromDouble(val);
}

static PyObject* absolute_new(PyTypeObject *, PyObject *args, PyObject *kwargs) {
	long countArgs = Py_SIZE(args);
	if (countArgs > 1) {
		PyErr_SetString(PyExc_TypeError, "absolute() expected 1 optional argument");
		return nullptr;
	}

	PyObject *arg = countArgs ? PySequence_Fast_GET_ITEM(args, 0) : nullptr;
	return absolute_new_imp(arg, kwargs);
}

static PyObject* absolute_vectorcall(PyObject *, PyObject *const *args, size_t nargsf, PyObject *kwargs) {
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	if (nargs > 1) {
		PyErr_SetString(PyExc_TypeError, "absolute() expected 1 optional argument");
		return nullptr;
	}

	PyObject *arg = nargs ? args[0] : nullptr;
	return absolute_new_imp(arg, kwargs);
}

void _PyFloat_ExactDealloc(PyObject *op);
static void absolute_dealloc(PyObject *obj) {
	obj->ob_type = &PyFloat_Type;
	_PyFloat_ExactDealloc(obj);
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


static PyObject* toStr(PyObject *self) {
	double value = PyFloat_AS_DOUBLE(self);
	std::string str = "absolute(" + std::to_string(value) + ")";

	PyObject *res = PyUnicode_FromStringAndSize(str.c_str(), long(str.size()));
	return res;
}

static Py_hash_t hash(PyObject *self) {
	double value = PyFloat_AS_DOUBLE(self);
	return _Py_HashDouble(self, value);
}


static PyMethodDef methods[] = {
    {"__reduce__", reduce, METH_NOARGS, "helper for pickle"},
    {"__round__", nullptr, 0, ""},                             //see PyAbsolute_PreInit
    {nullptr, nullptr, 0, nullptr}
};

#define CONVERT_TO_DOUBLE(obj, dbl) \
if (PyAbsolute_CheckExact(obj) || PyFloat_CheckExact(obj)) { \
	dbl = PyFloat_AS_DOUBLE(obj); \
}else if (PyLong_CheckExact(obj)) { \
	dbl = PyLong_AsDouble(obj); \
	if (PyErr_Occurred()) { \
	    PyErr_SetString(PyExc_OverflowError, "int too large to convert to absolute"); \
	    return nullptr; \
    } \
}else { \
	return PyErr_Format(PyExc_TypeError, \
	    "absolute.__%s__ expected absolute, float or int types, got %s", __func__, obj->ob_type->tp_name); \
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
static PyObject* rem(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	if (Math::doublesAreEq(b, 0)) {
		PyErr_SetString(PyExc_ZeroDivisionError, "absolute modulo");
		return nullptr;
	}

	double mod = std::fmod(a, b);
	if ((b < 0) != (mod < 0)) {
		mod += b;
	}
	return PyAbsolute_FromDouble(mod);
}

static PyObject* getFloorDiv(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	if (Math::doublesAreEq(b, 0)) {
		PyErr_SetString(PyExc_ZeroDivisionError, "absolute floor division by zero");
		return nullptr;
	}
	return PyAbsolute_FromDouble(std::floor(a / b));
}
static PyObject* getTrueDiv(PyObject *pyA, PyObject *pyB) {
	PREPARE_DOUBLES
	if (Math::doublesAreEq(b, 0)) {
		PyErr_SetString(PyExc_ZeroDivisionError, "absolute true division by zero");
		return nullptr;
	}
	return PyAbsolute_FromDouble(a / b);
}


static PyObject* neg(PyObject *pyA) {
	return PyAbsolute_FromDouble(-PyFloat_AS_DOUBLE(pyA));
}
static PyObject* pos(PyObject *pyA) {
	return Py_NewRef(pyA);
}
static PyObject* getAbs(PyObject *pyA) {
	double a = PyFloat_AS_DOUBLE(pyA);
	if (a >= 0) {
		return Py_NewRef(pyA);
	}
	return PyAbsolute_FromDouble(-a);
}
static int nonzero(PyObject *pyA) {
	return !Math::doublesAreEq(PyFloat_AS_DOUBLE(pyA), 0);
}


static PyObject* toLong(PyObject *pyA) {
	return PyLong_FromDouble(PyFloat_AS_DOUBLE(pyA));
}
static PyObject* toFloat(PyObject *pyA) {
	return PyFloat_FromDouble(PyFloat_AS_DOUBLE(pyA));
}


static PyNumberMethods asNumber = {
    add,         /* nb_add */
    sub,         /* nb_subtract */
    mul,         /* nb_multiply */
    rem,         /* nb_remainder */
    nullptr,     /* nb_divmod */
    nullptr,     /* nb_power */
    neg,         /* nb_negative */
    pos,         /* nb_positive */
    getAbs,      /* nb_absolute */
    nonzero,     /* nb_bool */
    nullptr,     /* nb_invert */
    nullptr,     /* nb_lshift */
    nullptr,     /* nb_rshift */
    nullptr,     /* nb_and */
    nullptr,     /* nb_xor */
    nullptr,     /* nb_or */
    toLong,      /* nb_int */
    nullptr,     /* nb_reserved */
    toFloat,     /* nb_float */
    nullptr,     /* nb_inplace_add */
    nullptr,     /* nb_inplace_subtract */
    nullptr,     /* nb_inplace_multiply */
    nullptr,     /* nb_inplace_remainder */
    nullptr,     /* nb_inplace_power */
    nullptr,     /* nb_inplace_lshift */
    nullptr,     /* nb_inplace_rshift */
    nullptr,     /* nb_inplace_and */
    nullptr,     /* nb_inplace_xor */
    nullptr,     /* nb_inplace_or */
    getFloorDiv, /* nb_floor_divide */
    getTrueDiv,  /* nb_true_divide */
    nullptr,     /* nb_inplace_floor_divide */
    nullptr,     /* nb_inplace_true_divide */
    nullptr,     /* nb_index */
    nullptr,     /* nb_matrix_multiply */
    nullptr,     /* nb_inplace_matrix_multiply */
};


static PyObject* richcompare(PyObject *pyA, PyObject *pyB, int op) {
	bool res = false;
	double a = PyFloat_AS_DOUBLE(pyA);
	double b;
	if (PyAbsolute_CheckExact(pyB) || PyFloat_CheckExact(pyB)) {
		b = PyFloat_AS_DOUBLE(pyB);
	}else if (PyLong_CheckExact(pyB)) {
		b = PyLong_AsDouble(pyB);
		if (PyErr_Occurred()) {
			PyErr_Clear();
			if (_PyLong_Sign(pyB) < 0) {
				//b is very small
				     if (op == Py_LT) res = false;
				else if (op == Py_LE) res = false;
				else if (op == Py_EQ) res = false;
				else if (op == Py_NE) res = true;
				else if (op == Py_GT) res = true;
				else if (op == Py_GE) res = true;
			}else {
				//b is very big
				     if (op == Py_LT) res = true;
				else if (op == Py_LE) res = true;
				else if (op == Py_EQ) res = false;
				else if (op == Py_NE) res = true;
				else if (op == Py_GT) res = false;
				else if (op == Py_GE) res = false;
			}

			PyObject *pyRes = res ? Py_True : Py_False;
			Py_INCREF(pyRes);
			return pyRes;
		}
	}else {
		Py_RETURN_NOTIMPLEMENTED;
	}

	     if (op == Py_LT) res = a < b;
	else if (op == Py_LE) res = a <= b;
	else if (op == Py_EQ) res = Math::doublesAreEq(a, b);
	else if (op == Py_NE) res = !Math::doublesAreEq(a, b);
	else if (op == Py_GT) res = a > b;
	else if (op == Py_GE) res = a >= b;

	PyObject *pyRes = res ? Py_True : Py_False;
	Py_INCREF(pyRes);
	return pyRes;
}


PyTypeObject PyAbsolute_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "absolute",
    sizeof(PyAbsoluteObject),
    0,
    absolute_dealloc,                 /* tp_dealloc */
    0,                                /* tp_vectorcall_offset */
    nullptr,                          /* tp_getattr */
    nullptr,                          /* tp_setattr */
    nullptr,                          /* tp_compare */
    toStr,                            /* tp_repr */
    &asNumber,                        /* tp_as_number */
    nullptr,                          /* tp_as_sequence */
    nullptr,                          /* tp_as_mapping */
    hash,                             /* tp_hash */
    nullptr,                          /* tp_call */
    toStr,                            /* tp_str */
    PyObject_GenericGetAttr,          /* tp_getattro */
    nullptr,                          /* tp_setattro */
    nullptr,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,               /* tp_flags */
    "Absolute coordinate in float.",  /* tp_doc */
    nullptr,                          /* tp_traverse */
    nullptr,                          /* tp_clear */
    richcompare,                      /* tp_richcompare */
    0,                                /* tp_weaklistoffset */
    nullptr,                          /* tp_iter */
    nullptr,                          /* tp_iternext */
    methods,                          /* tp_methods */
    nullptr,                          /* tp_members */
    nullptr,                          /* tp_getset */
    nullptr,                          /* tp_base */
    nullptr,                          /* tp_dict */
    nullptr,                          /* tp_descr_get */
    nullptr,                          /* tp_descr_set */
    0,                                /* tp_dictoffset */
    nullptr,                          /* tp_init */
    nullptr,                          /* tp_alloc */
    absolute_new,                     /* tp_new */
    nullptr,                          /* tp_free */
    nullptr,                          /* tp_is_gc */
    nullptr,                          /* tp_bases */
    nullptr,                          /* tp_mro */
    nullptr,                          /* tp_cache */
    nullptr,                          /* tp_subclasses */
    nullptr,                          /* tp_weaklist */
    nullptr,                          /* tp_del */
    0,                                /* tp_version_tag */
    nullptr,                          /* tp_finalize */
    absolute_vectorcall,              /* tp_vectorcall */
};


}
