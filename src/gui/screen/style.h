#ifndef STYLE_H
#define STYLE_H

#include <string>
#include <map>

#include <Python.h>

#include "parser/node.h"


struct StyleStruct {
	std::string name;
	PyObject *pyStyle;
	std::map<std::string, PyObject*> props;

	explicit StyleStruct(PyObject *style, const std::string &name);
	StyleStruct(const StyleStruct&) = delete;
	~StyleStruct();
};

class Style {
public:
	static void destroyAll();

	static const StyleStruct* getByName(const Node *node, const std::string &name);
	static const StyleStruct* getByNode(const Node *node, PyObject *style = nullptr);

	static PyObject* getProp(const StyleStruct *style, const std::string &propName);

	static void execAction(const std::string &fileName, size_t numLine,
	                       const StyleStruct *style, const std::string &propName);
};

#endif // STYLE_H
