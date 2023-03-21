#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
	static void init();
	static std::string get(const std::string &name);
	static void set(const std::string &name, const std::string &value, const std::string &comment = "");
	static void save();

	static void setDefaultScaleQuality();
	static bool setScaleQuality(std::string value);
	static std::string getScaleQuality();
};

#endif // CONFIG_H
