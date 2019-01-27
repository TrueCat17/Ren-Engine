#ifndef LOGGER_H
#define LOGGER_H

class String;

class Logger {
public:
	static void init();
	static void log(const String& str);
	static void logEvent(const String &event, int time, bool lastInGroup = false);
};

#endif // LOGGER_H
