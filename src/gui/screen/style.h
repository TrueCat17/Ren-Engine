#ifndef STYLE_H
#define STYLE_H

#include <map>

#include "utils/utils.h"

class Style {
private:
	static std::map<String, Style*> styles;

	std::map<String, std::pair<bool, String>> props;

public:
	static void destroyAll();
	static String getProp(const String &styleName, const String &propName);
};

#endif // STYLE_H
