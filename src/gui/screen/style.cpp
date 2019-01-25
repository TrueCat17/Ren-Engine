#include "style.h"

#include <vector>

#include "media/py_utils.h"
#include "utils/utils.h"


std::map<String, Style*> Style::styles;
void Style::destroyAll() {
	for (auto i : styles) {
		Style *style = i.second;
		delete style;
	}
	styles.clear();
}
Style::~Style() {
	for (auto &i : props) {
		PyObject *prop = i.second;
		Py_DECREF(prop);
	}
}


PyObject* Style::getProp(const String &styleName, const String &propName) {
	std::map<String, Style*>::const_iterator stylesIt = styles.find(styleName);
	if (stylesIt != styles.end()) {
		Style *style = stylesIt->second;
		std::map<String, PyObject*>::const_iterator styleIt = style->props.find(propName);
		if (styleIt != style->props.end()) return styleIt->second;
	}

	std::lock_guard g(PyUtils::pyExecMutex);

	PyObject *pyStyles = PyDict_GetItemString(PyUtils::global, "style");
	if (!pyStyles) {
		Utils::outMsg("Style::getProp", "Объект <style> не существует");
		Py_RETURN_NONE;
	}

	PyObject *pyStyle = PyObject_GetAttrString(pyStyles, styleName.c_str());
	if (!pyStyle || pyStyle == Py_None) {
		Utils::outMsg("Style::getProp", "Стиль <" + styleName + "> не существует");
		Py_RETURN_NONE;
	}

	PyObject *pyProp = PyObject_GetAttrString(pyStyle, propName.c_str());
	Py_INCREF(pyProp);

	if (stylesIt == styles.end()) {
		styles[styleName] = new Style();
		stylesIt = styles.find(styleName);
	}
	Style *style = stylesIt->second;
	style->props[propName] = pyProp;

	return pyProp;
}
