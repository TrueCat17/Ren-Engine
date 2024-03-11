#ifndef MODS_H
#define MODS_H

#include <Python.h>

class Mods {
public:
	static void init();          //read all names of all mods

	static void clearList();
	static PyObject* getList();  //[prepare and] return prepared tuple
};

#endif // MODS_H
