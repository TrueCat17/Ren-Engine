#include "style.h"

#include <set>

#include "media/py_utils.h"
#include "utils/scope_exit.h"
#include "utils/utils.h"


Style::Style(PyObject *style, const std::string &name):
    pyStyle(style),
    name(name)
{
	Py_INCREF(style);
	if (style == Py_None) return;

	PyObject *dict = PyObject_GetAttrString(style, "__dict__");
	if (!dict || !PyDict_CheckExact(dict)) {
		Utils::outMsg("Style::Style", "Expected type(obj.__dict__) is dict");
		Py_XDECREF(dict);
		return;
	}

	Py_ssize_t i = 0;
	PyObject *key, *value;
	while (PyDict_Next(dict, &i, &key, &value)) {
		if (PyUnicode_CheckExact(key)) {
			Py_INCREF(value);
			props[PyUtils::objToStr(key)] = value;
		}
	}

	Py_DECREF(dict);
}

Style::~Style() {
	Py_DECREF(pyStyle);
	for (auto &i : props) {
		PyObject *prop = i.second;
		Py_DECREF(prop);
	}
}



static std::map<std::string, const Style*> stylesByName;
static std::map<PyObject*, const Style*> stylesByPy;
void StyleManager::destroyAll() {
	std::set<const Style*> styles;
	for (auto &[name, style] : stylesByName) {
		styles.insert(style);
	}
	for (auto &[obj, style] : stylesByPy) {
		styles.insert(style);
	}

	PyUtils::callInPythonThread([&]() {
		for (const Style* style : styles) {
			delete style;
		}
		stylesByName.clear();
		stylesByPy.clear();
	});
}


//called from python thread
const Style* StyleManager::getByName(const Node *node, const std::string &name) {
	auto it = stylesByName.find(name);
	if (it != stylesByName.end()) {
		return it->second;
	}

	const Style *res;
	PyObject *pyStyle = PyUtils::execRetObj(node->getFileName(), node->getNumLine(), "style." + name);
	if (!pyStyle || pyStyle == Py_None) {
		Utils::outError("StyleManager::getByName", "Style <%> is not defined\n%", name, node->getPlace());
		res = new Style(Py_None, "None");
	}else {
		res = new Style(pyStyle, name);
	}
	Py_XDECREF(pyStyle);

	return stylesByName[name] = res;
}

//called from python thread
const Style* StyleManager::getByObject(const Child* obj, PyObject *style) {
	if (style) {
		auto it = stylesByPy.find(style);
		if (it != stylesByPy.end()) {
			return it->second;
		}
	}

	const Style *res;

	const Node *node = obj->node;
	const Node *propStyle = node->getProp("style");
	if (!propStyle && !style) {
		const Node *propHas = node->getProp("has");
		const std::string &styleName = propHas ? propHas->params : node->command;
		return getByName(node, styleName);
	}


	ScopeExit se([&]() { Py_DECREF(style); });
	se.enable = false;

	if (!style) {
		if (propStyle->screenNum != uint32_t(-1)) {//not const prop
			style = PySequence_Fast_GET_ITEM(obj->props, propStyle->screenNum);
		}else {
			style = PyUtils::execRetObj(node->getFileName(), node->getNumLine(), propStyle->params);
			if (style) {
				se.enable = true;
			}else {
				style = Py_None;
			}
		}
		auto it = stylesByPy.find(style);
		if (it != stylesByPy.end()) {
			return it->second;
		}
	}

	if (PyUnicode_CheckExact(style)) {
		res = getByName(node, PyUtils::objToStr(style));
	}else {
		std::string place = propStyle->getFileName() + ":" + std::to_string(propStyle->getNumLine());
		std::string name = "unknown (" + place + ")";
		res = new Style(style, name);
	}

	return stylesByPy[style] = res;
}


PyObject* StyleManager::getProp(const Style *style, const std::string &propName) {
	auto it = style->props.find(propName);
	if (it != style->props.end()) {
		return it->second;
	}
	return Py_None;
}

void StyleManager::execAction(const std::string &fileName, uint32_t numLine, const Style *style, const std::string &propName) {
	PyUtils::callInPythonThread([&]() {
		PyDict_SetItemString(PyUtils::global, "_SL_last_style", style->pyStyle);
		PyUtils::exec(fileName, numLine, "exec_funcs(_SL_last_style." + propName + ")");
	});
}
