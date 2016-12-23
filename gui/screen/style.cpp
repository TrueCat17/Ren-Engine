#include "style.h"

#include <vector>

#include "gv.h"

#include "utils/utils.h"


std::map<String, Style*> Style::styles;

void Style::updateAll() {
	if (!GV::inGame || Utils::execPython("globals().has_key('get_styles')", true) == "False") return;

	String stylesStr = Utils::execPython("get_styles()", true);
	std::vector<String> stylesVec = stylesStr.split(' ');

	for (String style : stylesVec) {
		if (!styles[style]) {
			styles[style] = new Style();
		}

		String propsStr = Utils::execPython("get_style_props('" + style + "')", true);
		std::vector<String> propsVec = propsStr.split(' ');

		std::map<String, String> &props = styles[style]->props;
		for (String &prop : propsVec) {
			props[prop] = Utils::execPython("style." + style + "." + prop, true);
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
		Utils::outMsg("Style::getProp", "Стиль <" + styleName + "> не существует");
		return "";
	}

	Style *style = styles[styleName];
	return style->props[propName];
}
