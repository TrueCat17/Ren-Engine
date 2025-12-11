#ifndef PYUTILS_H
#define PYUTILS_H

#include <functional>
#include <string>

#include <Python.h>


/* Python works in its own thread
 * All Python actions (running code and calling C API) are moved to this thread
*/

class PyUtils {
public:
	static PyObject *global;
	static PyObject *tuple1;

	static PyObject *marshalDumps;
	static PyObject *marshalLoads;

	static void init();
	static void callInPythonThread(const std::function<void()> &func);

	static void finalizeInterpreter();
	static void initInterpreter();

	static PyObject *getCompileObject(const std::string &code, const std::string &fileName, uint32_t numLine);
	static bool isConstExpr(const std::string &code);

	static std::string exec(const std::string &fileName, uint32_t numLine, const std::string &code, bool retRes = false);
	static std::string execWithSetTmp(const std::string &fileName, uint32_t numLine,
	                                  const std::string &code, const std::string &tmp,
	                                  bool retRes = false);

	static PyObject* execRetObj(const std::string &fileName, uint32_t numLine, const std::string &code);//returns new ref!!!
	static void errorProcessing(const std::string &code);

	static std::string objToStr(PyObject *obj);

	static bool isIdentifier(std::string_view s);
};


struct PyExecConstExprString {
	char data[64] = "CPP_EMBED: ";

	operator std::string() {
		return data;
	}
};

constexpr static PyExecConstExprString getBaseName(const char* path) {
	PyExecConstExprString res;
	char* data = res.data;

	char *end = data;
	while (*end) {
		++end;
	}

	const char *fileName = path;
	while (*path) {
		if (*path++ == '/') {
			fileName = path;
		}
	}

	while (*fileName) {
		*end++ = *fileName++;
	}
	*end = '\0';

	return res;
}

#define pyExecFromCpp(code) \
	PyUtils::exec(getBaseName(__FILE__), __LINE__, code, false)
#define pyExecFromCppWithRes(code) \
	PyUtils::exec(getBaseName(__FILE__), __LINE__, code, true)

#define pyExecFromCppWithSetTmp(code, tmp) \
	PyUtils::execWithSetTmp(getBaseName(__FILE__), __LINE__, code, tmp, false)
#define pyExecFromCppWithSetTmpWithRes(code, tmp) \
	PyUtils::execWithSetTmp(getBaseName(__FILE__), __LINE__, code, tmp, true)


#endif // PYUTILS_H
