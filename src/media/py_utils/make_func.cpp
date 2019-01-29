#include "make_func.h"


std::vector<PyWrapperBase> &getPyWrappers() {
	static std::vector<PyWrapperBase> vec;
	return vec;
}
void clearPyWrappers() {
	getPyWrappers().clear();
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
