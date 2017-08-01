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
	if (styles.find(styleName) == styles.end()) {
		String styleThereIs = PyUtils::exec("CPP_EMBED: style.cpp", __LINE__, "'" + styleName + "' in style.__dict__", true);

		if (styleThereIs != "True") {
			Utils::outMsg("Style::getProp", "Стиль <" + styleName + "> не существует");
			return "";
		}
		styles[styleName] = new Style();
	}
	Style *style = styles[styleName];

	if (style->props.find(propName) == style->props.end() || !style->props[propName].first) {
		String propValue = PyUtils::exec("CPP_EMBED: style.cpp", __LINE__, "style." + styleName + "." + propName, true);
		style->props[propName] = std::pair<bool, String>(true, propValue);
	}

	return style->props[propName].second;
}
