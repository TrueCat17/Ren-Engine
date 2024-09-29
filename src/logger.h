#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
public:
	static void init();
	static void log(const std::string &str);
	static void logWithEnd(const std::string &str, const std::string &end);
	static void logEvent(const std::string &event, double time, bool lastInGroup = false);
};

#endif // LOGGER_H
