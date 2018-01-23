#ifndef PYUTILS_H
#define PYUTILS_H

#include <mutex>
#include <map>

#include <boost/python.hpp>
namespace py = boost::python;

#include "utils/string.h"

typedef std::tuple<const String, const String, int> PyCode;

class PyUtils {
private:
	static std::map<PyCode, PyCodeObject*> compiledObjects;
	static py::list getMouse();
	static py::list getLocalMouse();

public:
	static const String True;
	static const String False;
	static const String None;

	static std::mutex pyExecMutex;


	static PyCodeObject* getCompileObject(const String &code, const String &fileName, size_t numLine, bool lock = false);
	static bool isConstExpr(const String &code, bool checkSimple = true);
	static String exec(const String &fileName, size_t numLine, const String &code, bool retRes = false);
	static py::object execRetObj(const String &fileName, size_t numLine, const String &code);
	static void errorProcessing(const String &code);

	static bool isInt(const py::object &obj) { return PyInt_CheckExact(obj.ptr()) || PyLong_CheckExact(obj.ptr()); }
	static bool isFloat(const py::object &obj) { return PyFloat_CheckExact(obj.ptr()); }
	static bool isTuple(const py::object &obj) { return PyTuple_CheckExact(obj.ptr()); }
	static bool isList(const py::object &obj) { return PyList_CheckExact(obj.ptr()); }
	static const std::string getStr(const py::object &obj) {
		std::lock_guard<std::mutex> g(pyExecMutex);
		if (PyString_CheckExact(obj.ptr())) {
			const char *chars = PyString_AS_STRING(obj.ptr());
			return std::string(chars);
		}
		return py::extract<const std::string>(py::str(obj));
	}
	static double getDouble(const py::object &obj, bool isFloat) {
		if (isFloat)                     return PyFloat_AsDouble(obj.ptr());
		if (PyInt_CheckExact(obj.ptr())) return PyInt_AsLong(obj.ptr());
		return PyLong_AsDouble(obj.ptr());
	}


	py::object mainModule;
	py::object pythonGlobal;

	PyUtils();
	~PyUtils();
};

#endif // PYUTILS_H
