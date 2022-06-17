#include "make_func.h"

static void clearMethodDefs();

std::vector<PyWrapperBase> &getPyWrappers() {
	static std::vector<PyWrapperBase> vec;
	return vec;
}
void clearPyWrappers() {
	getPyWrappers().clear();
	clearMethodDefs();
}


PyObject* pyFuncDelegator(PyObject *indexObj, PyObject *args) {
	if (!PyInt_CheckExact(indexObj)) {
		PyErr_SetString(PyExc_SystemError, "Error: index is not int");
		return nullptr;
	}

	std::vector<PyWrapperBase> &wrappers = getPyWrappers();
	size_t index = size_t(PyInt_AS_LONG(indexObj));
	if (index >= wrappers.size()) {
		std::string err = "wrapperIndex " + std::to_string(index) + " >= " + std::to_string(wrappers.size());
		PyErr_SetString(PyExc_SystemError, err.c_str());
		return nullptr;
	}

	const PyWrapperBase &wrapper = wrappers[index];
	return wrapper.call(wrapper, args);
}


static std::vector<PyMethodDef*> methodDefs;

PyMethodDef* getMethodDef(const char* name) {
	char *nameCopy = new char[strlen(name) + 1];
	strcpy(nameCopy, name);

	PyMethodDef *res = new PyMethodDef{ nameCopy, pyFuncDelegator, METH_VARARGS, nullptr };
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
