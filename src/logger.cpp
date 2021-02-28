#include "logger.h"

#include <ctime>
#include <fstream>

#include "utils/file_system.h"


static std::ofstream out;

void Logger::init() {
	if (!FileSystem::exists("../var")) {
		FileSystem::createDirectory("../var");
	}
	out.open("../var/log.txt");

	time_t seconds = std::time(nullptr);
	tm *timeInfo = std::localtime(&seconds);
	log(std::asctime(timeInfo));
}

void Logger::log(const std::string &str) {
	const size_t size = size_t(out.tellp());
	const size_t maxSize = 100 << 10;//100 kB
	if (size > maxSize) return;

	out << str << '\n';
	out.flush();
}
void Logger::logEvent(const std::string &event, double time, bool lastInGroup) {
	out << event << ": " << int(time * 1000) << " ms\n";
	if (lastInGroup) {
		out << "\n\n";
	}
	out.flush();
}
