#ifndef STYLE_H
#define STYLE_H

#include <map>

#include <Python.h>

#include "utils/string.h"


class Style {
private:
	static std::map<String, Style*> styles;

	std::map<String, PyObject*> props;

public:
	static void destroyAll();
	static PyObject* getProp(const String &styleName, const String &propName);

	~Style();
};

#endif // STYLE_H
