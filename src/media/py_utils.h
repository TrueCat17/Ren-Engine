#ifndef PYUTILS_H
#define PYUTILS_H

#include <map>

#include <boost/python.hpp>
namespace py = boost::python;

class String;

class PyUtils {
private:
	static std::map<String, PyCodeObject*> compiledObjects;

public:
	static PyCodeObject* getCompileObject(const String &code);
	static String exec(String code, bool retRes = false);

	py::object mainModule;
	py::object pythonGlobal;

	PyUtils();
	~PyUtils();
};

#endif // PYUTILS_H
