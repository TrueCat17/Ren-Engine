#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>


class String;

class Logger {
private:
	static std::ofstream out;

public:
	static void init();
	static void log(const String& str);
	static void logEvent(const String &event, int time, bool lastInGroup = false);
};

#endif // LOGGER_H
