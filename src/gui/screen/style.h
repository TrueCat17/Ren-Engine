#ifndef STYLE_H
#define STYLE_H

#include <map>
#include <string>

#include <Python.h>


class Style {
private:
	std::map<std::string, PyObject*> props;

public:
	static void destroyAll();
	static PyObject* getProp(const std::string &styleName, const std::string &propName);

	~Style();
};

#endif // STYLE_H
