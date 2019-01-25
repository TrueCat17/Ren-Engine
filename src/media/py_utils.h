#ifndef PYUTILS_H
#define PYUTILS_H

#include <mutex>
#include <map>

#include <Python.h>

#include "utils/string.h"

typedef std::tuple<const String, const String, int> PyCode;

class PyUtils {
private:
	static std::map<String, PyObject*> constObjects;
	static std::map<PyCode, PyCodeObject*> compiledObjects;

public:	
	static PyUtils *obj;


	static const String True;
	static const String False;
	static const String None;

	static PyObject *global;
	static PyObject *tuple1;
	static std::recursive_mutex pyExecMutex;


	static PyCodeObject* getCompileObject(const String &code, const String &fileName, size_t numLine);
	static bool isConstExpr(const String &code, bool checkSimple = true);
	static String exec(const String &fileName, size_t numLine, const String &code, bool retRes = false);
	static PyObject* execRetObj(const String &fileName, size_t numLine, const String &code);
	static void errorProcessing(const String &code);

	static String getStr(PyObject *obj) {
		std::lock_guard g(pyExecMutex);
		if (PyString_CheckExact(obj)) {
			const char *chars = PyString_AS_STRING(obj);
			return String(chars);
		}
		PyObject *tmpStr = PyObject_Str(obj);
		if (!tmpStr) {
			return "Error on str(" + String(obj->ob_type->tp_name) + ")";
		}
		String res = PyString_AS_STRING(tmpStr);
		Py_DECREF(tmpStr);
		return res;
	}


	PyUtils();
	~PyUtils();
};

#endif // PYUTILS_H
