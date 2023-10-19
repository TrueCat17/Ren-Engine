#ifndef PYUTILS_H
#define PYUTILS_H

#include <mutex>
#include <string>

#include <Python.h>


class PyUtils {
public:
	static PyObject *global;
	static PyObject *tuple1;
	static std::recursive_mutex pyExecMutex;


	static PyObject *getCompileObject(const std::string &code, const std::string &fileName, uint32_t numLine);
	static bool isConstExpr(const std::string &code, bool checkSimple = true);
	static std::string exec(const std::string &fileName, uint32_t numLine, const std::string &code, bool retRes = false);
	static PyObject* execRetObj(const std::string &fileName, uint32_t numLine, const std::string &code);
	static void errorProcessing(const std::string &code);

	static void init();

	static std::string objToStr(PyObject *obj);
};

#endif // PYUTILS_H
