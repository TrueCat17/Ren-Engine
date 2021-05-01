#ifndef MAKE_FUNC_H
#define MAKE_FUNC_H

#include <vector>
#include <utility>

#include <Python.h>

#include "convert_to_py.h"
#include "convert_from_py.h"



template<typename T>
struct remove_rc {
	typedef T type;
};
template<typename T>
struct remove_rc<const T> {
	typedef T type;
};
template<typename T>
struct remove_rc<const T&> {
	typedef T type;
};
template<typename T>
struct remove_rc<T&> {
	//error
};


class PyWrapperBase {
public:
	PyObject* (*call)(const PyWrapperBase&, PyObject*);
	void *funcPtr;
	const char *name;

	PyWrapperBase(PyObject* (*call)(const PyWrapperBase&, PyObject*), void *funcPtr, const char *name):
		call(call),
		funcPtr(funcPtr),
		name(name)
	{}
};


std::vector<PyWrapperBase> &getPyWrappers();
void clearPyWrappers();


PyObject* pyFuncDelegator(PyObject *indexObj, PyObject *args);



template<typename Ret, typename... Args>
class PyWrapper: public PyWrapperBase {
public:
	PyWrapper(const char *name, Ret(*funcPtr)(Args...)):
		PyWrapperBase(call, reinterpret_cast<void*>(funcPtr), name)
	{}

private:
	static constexpr size_t COUNT_ARGS = sizeof...(Args);

	static PyObject* call(const PyWrapperBase &wrapper, PyObject *args) {
		return helper(wrapper, args, std::make_index_sequence<COUNT_ARGS>());
	}

	template<size_t ...Indexes>
	static PyObject* helper(const PyWrapperBase &wrapper, PyObject *args, std::index_sequence<Indexes...>) {
		if (Py_SIZE(args) != COUNT_ARGS) {
			std::string err = std::string(wrapper.name) + "() takes exactly ";
			if constexpr (COUNT_ARGS == 1) {
				err += "1 argument";
			}else {
				err += std::to_string(COUNT_ARGS) + " arguments";
			}
			err += " (" + std::to_string(Py_SIZE(args)) + " given)";

			PyErr_SetString(PyExc_TypeError, err.c_str());
			return nullptr;
		}

		try {
			Ret (*typedFunc)(Args...) = reinterpret_cast<Ret(*)(Args...)>(wrapper.funcPtr);

			if constexpr (std::is_same<Ret, void>::value) {
				typedFunc(convertFromPy<typename remove_rc<Args>::type>(PyTuple_GET_ITEM(args, Indexes), wrapper.name, Indexes)...);
				Py_RETURN_NONE;
			}else {
				typename remove_rc<Ret>::type res = typedFunc(convertFromPy<typename remove_rc<Args>::type>(PyTuple_GET_ITEM(args, Indexes), wrapper.name, Indexes)...);
				return convertToPy(res);
			}
		}catch (PyConvertError&) {
			return nullptr;
		}
	}
};


template<typename Ret, typename ...Args>
PyObject* makeFuncImpl(const char *name, Ret(*func)(Args...)) {
	std::vector<PyWrapperBase> &pyWrappers = getPyWrappers();

	PyWrapper<Ret, Args...> wrapper(name, func);
	pyWrappers.push_back(wrapper);

	static PyMethodDef tmp{"delegator", pyFuncDelegator, METH_VARARGS, nullptr};
	return PyCFunction_NewEx(&tmp, PyInt_FromLong(long(pyWrappers.size() - 1)), nullptr);
}


#define makePyFunc(...) makeFuncImpl(#__VA_ARGS__, __VA_ARGS__)


#endif // MAKE_FUNC_H
