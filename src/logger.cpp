#include "logger.h"

#include <ctime>

#include "utils/string.h"
#include "utils/utils.h"


std::ofstream Logger::out;

void Logger::init() {
	out.open(Utils::ROOT + "log.txt");

	time_t seconds = std::time(nullptr);
	tm *timeInfo = std::localtime(&seconds);
	out << std::asctime(timeInfo) << '\n';
}

void Logger::log(const String &str) {
	out << str << '\n';
}
void Logger::logEvent(const String &event, int time, bool lastInGroup) {
	out << event << ": " << time << " ms\n";
	if (lastInGroup) {
		out << '\n';
	}
}
