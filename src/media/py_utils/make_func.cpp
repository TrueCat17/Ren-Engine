#include "make_func.h"

static void clearMethodDefs();

static std::vector<PyWrapperBase> pyWrappers;
std::vector<PyWrapperBase> &getPyWrappers() {
	return pyWrappers;
}
void clearPyWrappers() {
	getPyWrappers().clear();
	clearMethodDefs();
}


static PyObject* pyFuncDelegator(PyObject *indexObj, PyObject *const *args, Py_ssize_t nargs) {
	if (!PyLong_CheckExact(indexObj)) {
		PyErr_SetString(PyExc_SystemError, "Error: index is not int");
		return nullptr;
	}

	std::vector<PyWrapperBase> &wrappers = getPyWrappers();
	size_t index = size_t(PyLong_AS_LONG(indexObj));
	if (index >= wrappers.size()) {
		return PyErr_Format(PyExc_SystemError, "wrapperIndex %zu >= %zu", index, wrappers.size());
	}

	const PyWrapperBase &wrapper = wrappers[index];
	return wrapper.call(wrapper, args, nargs);
}


static std::vector<PyMethodDef*> methodDefs;

PyMethodDef* getMethodDef(const char* name) {
	char *nameCopy = new char[strlen(name) + 1];
	strcpy(nameCopy, name);

	PyMethodDef *res = new PyMethodDef{ nameCopy, _PyCFunction_CAST(pyFuncDelegator), METH_FASTCALL, nullptr };
	methodDefs.push_back(res);
	return res;
}
static void clearMethodDefs() {
	for (PyMethodDef *def : methodDefs) {
		delete[] def->ml_name;
		delete def;
	}
	methodDefs.clear();
}
