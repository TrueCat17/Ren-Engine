#ifndef PYUTILS_H
#define PYUTILS_H

#include <map>

#include <boost/python.hpp>
namespace py = boost::python;

#include "utils/string.h"


struct PyCode {
private:
	String common;

public:
	String code;
	String fileName;
	size_t numLine;

	PyCode(String code, String fileName, size_t numLine):
		code(code),
		fileName(fileName),
		numLine(numLine)
	{
		common = code + fileName + String(numLine);
	}

	bool operator<(const PyCode& other) const {
		return common < other.common;
	}
};


class PyUtils {
private:
	static std::map<PyCode, PyCodeObject*> compiledObjects;

public:
	static PyCodeObject* getCompileObject(const String code, const String fileName, size_t numLine);
	static String exec(const String &fileName, size_t numLine, String code, bool retRes = false);

	py::object mainModule;
	py::object pythonGlobal;

	PyUtils();
	~PyUtils();
};

#endif // PYUTILS_H
