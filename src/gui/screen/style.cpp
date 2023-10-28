#include "style.h"

#include "media/py_utils.h"
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
		return;
	}

	Py_ssize_t i = 0;
	PyObject *key, *value;
	while (PyDict_Next(dict, &i, &key, &value)) {
		if (PyUnicode_CheckExact(key)) {
			Py_INCREF(value);
			props[PyUnicode_AsUTF8(key)] = value;
		}
	}
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
	std::lock_guard g(PyUtils::pyExecMutex);

	for (auto &[name, style] : stylesByName) {
		delete style;
	}
	stylesByName.clear();
	stylesByPy.clear();
}


const Style* StyleManager::getByName(const Node *node, const std::string &name) {
	auto it = stylesByName.find(name);
	if (it != stylesByName.end()) {
		return it->second;
	}

	std::lock_guard g(PyUtils::pyExecMutex);

	const Style *res;
	PyObject *pyStyle = PyUtils::execRetObj(node->getFileName(), node->getNumLine(), "style." + name);
	if (!pyStyle || pyStyle == Py_None) {
		Utils::outMsg("StyleManager::getByName", "Style <" + name + "> is not defined");
		res = new Style(Py_None, "None");
	}else {
		res = new Style(pyStyle, name);
	}

	return stylesByName[name] = res;
}

const Style* StyleManager::getByObject(const Child* obj, PyObject *style) {
	if (style) {
		auto it = stylesByPy.find(style);
		if (it != stylesByPy.end()) {
			return it->second;
		}
	}

	std::lock_guard g(PyUtils::pyExecMutex);

	const Node *node = obj->node;
	const Node *propStyle = node->getProp("style");
	if (propStyle || style) {
		if (!style) {
			if (propStyle->screenNum != uint32_t(-1)) {//not const prop
				style = PySequence_Fast_GET_ITEM(obj->props, propStyle->screenNum);
			}else {
				style = PyUtils::execRetObj(node->getFileName(), node->getNumLine(), propStyle->params);
			}
		}

		const Style *res;
		if (PyUnicode_CheckExact(style)) {
			res = getByName(node, PyUnicode_AsUTF8(style));
		}else {
			std::string place = propStyle->getFileName() + ":" + std::to_string(propStyle->getNumLine());
			std::string name = "unknown (" + place + ")";
			res = new Style(style, name);
		}
		return stylesByPy[style] = res;
	}

	const Node *propHas = node->getProp("has");
	std::string styleName = propHas ? propHas->params : node->command;
	return stylesByPy[style] = getByName(node, styleName);
}


PyObject* StyleManager::getProp(const Style *style, const std::string &propName) {
	auto it = style->props.find(propName);
	if (it != style->props.end()) {
		return it->second;
	}
	return Py_None;
}

void StyleManager::execAction(const std::string &fileName, uint32_t numLine, const Style *style, const std::string &propName) {
	std::lock_guard g(PyUtils::pyExecMutex);

	PyDict_SetItemString(PyUtils::global, "_SL_last_style", style->pyStyle);
	PyUtils::exec(fileName, numLine, "exec_funcs(_SL_last_style." + propName + ")");
}
