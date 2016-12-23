#ifndef STYLE_H
#define STYLE_H

#include <map>

#include "utils/string.h"

class Style {
private:
	static std::map<String, Style*> styles;

	std::map<String, String> props;

public:
	static void updateAll();
	static void destroyAll();

	static String getProp(const String &styleName, const String &propName);
};

#endif // STYLE_H
