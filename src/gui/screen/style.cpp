#include "style.h"

#include <vector>

#include "gv.h"
#include "media/py_utils.h"
#include "utils/utils.h"

namespace py = boost::python;

std::map<String, Style*> Style::styles;
void Style::destroyAll() {
	for (auto i : styles) {
		Style *style = i.second;
		delete style;
	}
	styles.clear();
}


py::object Style::getProp(const String &styleName, const String &propName) {
	try {
		py::object pyStyles;
		py::object pyStyle;

		std::map<String, Style*>::const_iterator stylesIt = styles.find(styleName);
		if (stylesIt == styles.end()) {
			{
				std::lock_guard<std::mutex> g(GV::pyUtils->pyExecMutex);
				pyStyles = GV::pyUtils->pythonGlobal["style"];
				pyStyle = pyStyles[styleName.c_str()];
			}
			if (pyStyle.is_none()) {
				Utils::outMsg("Style::getProp", "Стиль <" + styleName + "> не существует");
				return py::object();
			}

			styles[styleName] = new Style();
			stylesIt = styles.find(styleName);
		}
		Style *style = stylesIt->second;

		std::map<String, py::object>::const_iterator styleIt = style->props.find(propName);
		if (styleIt == style->props.end()) {
			std::lock_guard<std::mutex> g(GV::pyUtils->pyExecMutex);
			if (pyStyle.is_none()) {
				if (pyStyles.is_none()) {
					pyStyles = GV::pyUtils->pythonGlobal["style"];
				}
				pyStyle = pyStyles[styleName.c_str()];
			}
			py::object value = pyStyle[propName.c_str()];
			style->props[propName] = value;
			styleIt = style->props.find(propName);
		}

		return styleIt->second;
	}catch (py::error_already_set) {
		const String code = "style." + styleName + "." + propName;
		PyUtils::errorProcessing("EMBED_CPP: Style::getProp:\n" + code);
		Utils::outMsg("Style::getProp", "Ошибка при попытке вычислить " + code);
	}
	return py::object();
}
