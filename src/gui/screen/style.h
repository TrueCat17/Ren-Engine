#ifndef STYLE_H
#define STYLE_H

#include <map>

#include <boost/python.hpp>

#include "utils/string.h"


class Style {
private:
	static std::map<String, Style*> styles;

	std::map<String, boost::python::object> props;

public:
	static void destroyAll();
	static boost::python::object getProp(const String &styleName, const String &propName);
};

#endif // STYLE_H
