#ifndef PYUTILS_H
#define PYUTILS_H

#include <mutex>
#include <map>

#include <Python.h>

#include "utils/string.h"

class PyUtils {
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


	PyUtils();
	~PyUtils();
};

#endif // PYUTILS_H
