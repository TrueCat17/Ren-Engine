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

public:
	static std::mutex pyExecMutex;

	static PyCodeObject* getCompileObject(const String &code, const String &fileName, size_t numLine);
	static bool isConstExpr(const String &code);
	static String exec(const String &fileName, size_t numLine, const String &code, bool retRes = false);
	static void errorProcessing(const String &code);

	py::object mainModule;
	py::object pythonGlobal;

	PyUtils();
	~PyUtils();
};

#endif // PYUTILS_H
