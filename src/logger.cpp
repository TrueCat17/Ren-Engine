#include "logger.h"

#include <ctime>

#include "utils/string.h"


std::ofstream Logger::out;

void Logger::init() {
	out.open("log.txt");

	time_t seconds = std::time(nullptr);
	tm *timeInfo = std::localtime(&seconds);
	log(std::asctime(timeInfo));
}

void Logger::log(const String &str) {
	out << str << '\n';
	out.flush();
}
void Logger::logEvent(const String &event, int time, bool lastInGroup) {
	out << event << ": " << time << " ms\n";
	if (lastInGroup) {
		out << "\n\n";
	}
	out.flush();
}
