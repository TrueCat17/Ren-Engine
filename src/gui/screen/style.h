#ifndef STYLE_H
#define STYLE_H

#include <map>

#include "utils/utils.h"

class Style {
private:
	static std::map<String, Style*> styles;

	std::map<String, py::object> props;

public:
	static void destroyAll();
	static py::object getProp(const String &styleName, const String &propName);
};

#endif // STYLE_H
