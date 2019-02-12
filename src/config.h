#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>

struct Param {
	std::string name;
	std::string value;
	std::string comment;
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
	static std::string get(const std::string &name);
	static void set(const std::string &name, const std::string &value, const std::string &comment = "");
	static void save();
};

#endif // CONFIG_H
