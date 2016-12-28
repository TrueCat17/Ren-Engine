#ifndef CONFIG_H
#define CONFIG_H

#include <vector>

#include "utils/string.h"

struct Param {
	String name;
	String value;
	String comment;
};

class Config {
private:
	static std::vector<Param> params;
	static bool initing;
	static bool changed;

	static void setDefault();
	static void load();

public:
	static void init();
	static String get(const String &name);
	static void set(const String &name, const String &value, const String &comment = "");
	static void save();
};

#endif // CONFIG_H
