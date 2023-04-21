#ifndef MAKE_FUNC_H
#define MAKE_FUNC_H

#include <string>
#include <tuple>
#include <vector>

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


class PyWrapperBase;
typedef PyObject* (*PyWrapperCallType)(const PyWrapperBase&, PyObject *const *args, Py_ssize_t nargs);

class PyWrapperBase {
public:
	PyWrapperCallType call;
	void *funcPtr;
	const char *name;

	PyWrapperBase(PyWrapperCallType call, void *funcPtr, const char *name):
	    call(call),
	    funcPtr(funcPtr),
	    name(name)
	{}
};


std::vector<PyWrapperBase> &getPyWrappers();
void clearPyWrappers();



template<typename Ret, typename... Args>
class PyWrapper: public PyWrapperBase {
public:
	PyWrapper(const char *name, Ret(*funcPtr)(Args...)):
	    PyWrapperBase(call, reinterpret_cast<void*>(funcPtr), name)
	{}

private:
	static constexpr size_t COUNT_ARGS = sizeof...(Args);

	static PyObject* call(const PyWrapperBase &wrapper, PyObject *const *pyArgs, Py_ssize_t nargs) {
		if (nargs != COUNT_ARGS) {
			return PyErr_Format(PyExc_TypeError,
			    "%s() takes exactly %zu argument%s (%zi given)",
			    wrapper.name, COUNT_ARGS, COUNT_ARGS == 1 ? "" : "s", nargs);
		}
		return helper(wrapper, pyArgs, std::make_index_sequence<COUNT_ARGS>());
	}

	template<size_t ...Indexes>
	static PyObject* helper(const PyWrapperBase &wrapper, PyObject *const *pyArgs, std::index_sequence<Indexes...>) {
		Ret (*typedFunc)(Args...) = reinterpret_cast<Ret(*)(Args...)>(wrapper.funcPtr);

		pyErrorFlag = false;
		std::tuple args = {
		    convertFromPy<typename remove_rc<Args>::type>(pyArgs[Indexes], wrapper.name, Indexes)...
		};
		if (pyErrorFlag) {
			return nullptr;
		}

		if constexpr (std::is_void_v<Ret>) {
			std::apply(typedFunc, args);
			Py_RETURN_NONE;
		}else {
			typename remove_rc<Ret>::type res = std::apply(typedFunc, args);
			if constexpr (std::is_arithmetic_v<Ret> || std::is_pointer_v<Ret>) {
				return convertToPy(res);
			}else {
				return convertToPy<const Ret&>(res);
			}
		}
	}
};

PyMethodDef* getMethodDef(const char* name);

template<typename Ret, typename ...Args>
PyObject* makeFuncImpl(const char *name, Ret(*func)(Args...), PyObject *moduleName = nullptr) {
	std::vector<PyWrapperBase> &pyWrappers = getPyWrappers();

	PyWrapper<Ret, Args...> wrapper(name, func);
	pyWrappers.push_back(wrapper);

	PyMethodDef *methodDef = getMethodDef(name);
	PyObject *res = PyCFunction_New(methodDef, PyLong_FromSize_t(pyWrappers.size() - 1));
	if (moduleName) {
		PyObject_SetAttrString(res, "__module__", moduleName);
	}
	return res;
}


#define makePyFunc(...) makeFuncImpl(#__VA_ARGS__, __VA_ARGS__)


#endif // MAKE_FUNC_H
