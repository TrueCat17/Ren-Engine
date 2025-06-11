#ifndef PY_CODE_DISK_CACHE_H
#define PY_CODE_DISK_CACHE_H

#include <string>

#include <Python.h>


class PyCodeDiskCache {
public:
	//call all funcs only from python-thread

	static void init();

	static PyObject* get(const std::string &code, const std::string &fileName, uint32_t numLine);
	static void set(const std::string &code, const std::string &fileName, uint32_t numLine, PyObject *pyCodeObject);

	static void checkSaving();
};

#endif // PY_CODE_DISK_CACHE_H
