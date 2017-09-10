#ifndef PYUTILS_H
#define PYUTILS_H

#include <mutex>
#include <map>

#include <boost/python.hpp>
namespace py = boost::python;

#include "utils/string.h"


struct PyCode {
public:
	String code;
	String fileName;
	size_t numLine;

	PyCode(const String &code, const String &fileName, size_t numLine):
		code(code),
		fileName(fileName),
		numLine(numLine)
	{}

	bool operator<(const PyCode& other) const {
		return std::tie(numLine, code, fileName) < std::tie(other.numLine, other.code, other.fileName);
	}
};


class PyUtils {
private:
	static std::map<PyCode, PyCodeObject*> compiledObjects;
	static py::list getMouse();

public:
	static std::mutex pyExecMutex;

	static PyCodeObject* getCompileObject(const String &code, const String &fileName, size_t numLine);
	static String exec(const String &fileName, size_t numLine, const String &code, bool retRes = false);
	static void errorProcessing(const String &code);

	py::object mainModule;
	py::object pythonGlobal;

	PyUtils();
	~PyUtils();
};

#endif // PYUTILS_H
