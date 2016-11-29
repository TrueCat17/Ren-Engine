#ifndef PYGUARD_H
#define PYGUARD_H

#include <map>

#include <boost/python.hpp>
namespace py = boost::python;

#include <utils/string.h>

class PyGuard {
private:
	static std::map<String, PyCodeObject*> compiledObjects;

public:
	static PyCodeObject* getCompileObject(const String &code);


	py::object mainModule;
	py::object pythonGlobal;

	PyGuard();
	~PyGuard();
};

#endif // PYGUARD_H
