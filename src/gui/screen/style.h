#ifndef STYLE_H
#define STYLE_H

#include <string>
#include <map>

#include <Python.h>

#include "parser/node.h"


struct Style {
	PyObject *pyStyle;
	std::string name;
	std::map<std::string, PyObject*> props;

	explicit Style(PyObject *style, const std::string &name);
	Style(const Style&) = delete;
	~Style();
};

class StyleManager {
public:
	static void destroyAll();

	static const Style* getByName(const Node *node, const std::string &name);
	static const Style* getByNode(const Node *node, PyObject *style = nullptr);

	static PyObject* getProp(const Style *style, const std::string &propName);

	static void execAction(const std::string &fileName, uint32_t numLine,
	                       const Style *style, const std::string &propName);
};

#endif // STYLE_H
