#include "style.h"

#include "media/py_utils.h"
#include "utils/utils.h"


StyleStruct::StyleStruct(PyObject *style, const std::string &name) {
	this->name = name;

	Py_INCREF(style);
	this->pyStyle = style;
	if (style == Py_None) return;

	PyObject *dict = PyObject_GetAttrString(style, "__dict__");
	if (!dict || !PyDict_CheckExact(dict)) {
		Utils::outMsg("StyleStruct::StyleStruct", "Expected type(obj.__dict__) is dict");
		return;
	}

	Py_ssize_t i = 0;
	PyObject *key, *value;
	while (PyDict_Next(dict, &i, &key, &value)) {
		if (PyString_CheckExact(key)) {
			Py_INCREF(value);
			props[PyString_AS_STRING(key)] = value;
		}
	}
}

StyleStruct::~StyleStruct() {
	Py_DECREF(pyStyle);
	for (auto &i : props) {
		PyObject *prop = i.second;
		Py_DECREF(prop);
	}
}



static std::map<std::string, const StyleStruct*> styleStructs;
static std::map<PyObject*, const StyleStruct*> stylesByPy;
void Style::destroyAll() {
	std::lock_guard g(PyUtils::pyExecMutex);

	for (auto &[name, style] : styleStructs) {
		delete style;
	}
	styleStructs.clear();
	stylesByPy.clear();
}


const StyleStruct* Style::getByName(const Node *node, const std::string &name) {
	auto it = styleStructs.find(name);
	if (it != styleStructs.end()) {
		return it->second;
	}

	std::lock_guard g(PyUtils::pyExecMutex);

	const StyleStruct *res;
	PyObject *pyStyle = PyUtils::execRetObj(node->getFileName(), node->getNumLine(), "style." + name);
	if (!pyStyle || pyStyle == Py_None) {
		Utils::outMsg("Style::getByName", "Style <" + name + "> is not defined");
		res = new StyleStruct(Py_None, "None");
	}else {
		res = new StyleStruct(pyStyle, name);
	}

	return styleStructs[name] = res;
}

const StyleStruct* Style::getByNode(const Node *node, PyObject *style) {
	if (style){
		auto it = stylesByPy.find(style);
		if (it != stylesByPy.end()) {
			return it->second;
		}
	}

	std::lock_guard g(PyUtils::pyExecMutex);

	const Node *propStyle = node->getProp("style");
	if (propStyle || style) {
		if (!style) {
			style = PyUtils::execRetObj(node->getFileName(), node->getNumLine(), propStyle->params);
		}

		const StyleStruct *res;
		if (PyString_CheckExact(style)) {
			res = getByName(node, PyString_AS_STRING(style));
		}else {
			std::string name = "unknown (" +
			        propStyle->getFileName() + ":" + std::to_string(propStyle->getNumLine()) +
			        ")";
			res = new StyleStruct(style, name);
		}
		return stylesByPy[style] = res;
	}

	const Node *propHas = node->getProp("has");
	std::string styleName = propHas ? propHas->params : node->command;
	return stylesByPy[style] = getByName(node, styleName);
}


PyObject* Style::getProp(const StyleStruct *style, const std::string &propName) {
	auto it = style->props.find(propName);
	if (it != style->props.end()) {
		return it->second;
	}
	return Py_None;
}

void Style::execAction(const std::string &fileName, size_t numLine, const StyleStruct *style, const std::string &propName) {
	std::lock_guard g(PyUtils::pyExecMutex);

	PyDict_SetItemString(PyUtils::global, "last_style", style->pyStyle);
	PyUtils::exec(fileName, numLine, "exec_funcs(last_style." + propName + ")");
}
