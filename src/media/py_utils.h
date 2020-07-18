#ifndef PYUTILS_H
#define PYUTILS_H

#include <mutex>

#include <Python.h>


class PyUtils {
public:
	static PyObject *global;
	static PyObject *tuple1;
	static PyObject *sysExcInfo;
	static PyObject *formatTraceback;
	static std::recursive_mutex pyExecMutex;


	static PyCodeObject* getCompileObject(const std::string &code, const std::string &fileName, size_t numLine);
	static bool isConstExpr(const std::string &code, bool checkSimple = true);
	static std::string exec(const std::string &fileName, size_t numLine, const std::string &code, bool retRes = false);
	static PyObject* execRetObj(const std::string &fileName, size_t numLine, const std::string &code);
	static void errorProcessing(const std::string &code);

	static void init();
};

#endif // PYUTILS_H
