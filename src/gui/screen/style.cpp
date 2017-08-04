#include "style.h"

#include <vector>

#include "gv.h"

#include "media/py_utils.h"
#include "utils/utils.h"


std::map<String, Style*> Style::styles;

void Style::disableAll() {
	for (auto i : styles) {
		Style *style = i.second;
		std::map<String, std::pair<bool, String>> &props = style->props;
		for (auto &j : props) {
			auto &pair = j.second;
			pair.first = false;
		}
	}
}
void Style::destroyAll() {
	for (auto i : styles) {
		Style *style = i.second;
		delete style;
	}
	styles.clear();
}

String Style::getProp(const String &styleName, const String &propName) {
	std::lock_guard<std::mutex> guard(GV::pyUtils->pyExecGuard);

	try {
		py::object styleObj = GV::pyUtils->pythonGlobal["style"];

		if (styles.find(styleName) == styles.end()) {
			py::object styleThereIs = styleObj[(styleName).c_str()];
			if (styleThereIs.is_none()) {
				Utils::outMsg("Style::getProp", "Стиль <" + styleName + "> не существует");
				return "";
			}
			styles[styleName] = new Style();
		}
		Style *style = styles[styleName];

		if (style->props.find(propName) == style->props.end() || !style->props[propName].first) {
			py::object needStyle = styleObj[styleName.c_str()];
			py::object prop = needStyle[propName.c_str()];

			String propValue = String(py::extract<const std::string>(py::str(prop)));

			style->props[propName] = std::pair<bool, String>(true, propValue);
		}

		return style->props[propName].second;
	}catch (py::error_already_set) {
		String code = "style." + styleName + "." + propName;
		PyUtils::errorProcessing("EMBED_CPP: Style::getProp:\n" + code);
		Utils::outMsg("Style::getProp", "Ошибка при попытке вычислить " + code);
	}
	return "None";
}
