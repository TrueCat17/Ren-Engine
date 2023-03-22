#ifndef MAKE_FUNC_H
#define MAKE_FUNC_H

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

	static PyObject* call(const PyWrapperBase &wrapper, PyObject *pyArgs) {
		return helper(wrapper, pyArgs, std::make_index_sequence<COUNT_ARGS>());
	}

	template<size_t ...Indexes>
	static PyObject* helper(const PyWrapperBase &wrapper, PyObject *pyArgs, std::index_sequence<Indexes...>) {
		if (Py_SIZE(pyArgs) != COUNT_ARGS) {
			std::string err = std::string(wrapper.name) + "() takes exactly ";
			if constexpr (COUNT_ARGS == 1) {
				err += "1 argument";
			}else {
				err += std::to_string(COUNT_ARGS) + " arguments";
			}
			err += " (" + std::to_string(Py_SIZE(pyArgs)) + " given)";

			PyErr_SetString(PyExc_TypeError, err.c_str());
			return nullptr;
		}

		Ret (*typedFunc)(Args...) = reinterpret_cast<Ret(*)(Args...)>(wrapper.funcPtr);

		pyErrorFlag = false;
		std::tuple args = {
		    convertFromPy<typename remove_rc<Args>::type>(PyTuple_GET_ITEM(pyArgs, Indexes), wrapper.name, Indexes)...
		};
		if (pyErrorFlag) {
			return nullptr;
		}

		if constexpr (std::is_same_v<Ret, void>) {
			std::apply(typedFunc, args);
			Py_RETURN_NONE;
		}else {
			typename remove_rc<Ret>::type res = std::apply(typedFunc, args);
			return convertToPy(res);
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
	PyObject *res = PyCFunction_NewEx(methodDef, PyInt_FromLong(long(pyWrappers.size() - 1)), nullptr);
	if (moduleName) {
		PyObject_SetAttrString(res, "__module__", moduleName);
	}
	return res;
}


#define makePyFunc(...) makeFuncImpl(#__VA_ARGS__, __VA_ARGS__)


#endif // MAKE_FUNC_H
