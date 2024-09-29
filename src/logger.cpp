#include "logger.h"

#include <ctime>
#include <fstream>

#include "utils/file_system.h"
#include "utils/string.h"


static std::ofstream out;

void Logger::init() {
	if (!FileSystem::exists("../var")) {
		FileSystem::createDirectory("../var");
	}
	out.open("../var/log.txt");

	time_t seconds = std::time(nullptr);
	tm *timeInfo = std::localtime(&seconds);

	char buffer[50];
	std::strftime(buffer, sizeof(buffer), "%Y.%m.%d-%H:%M:%S", timeInfo);
	logWithEnd(buffer, "\n\n");
}

void Logger::log(const std::string &str) {
	logWithEnd(str, "\n");
}
void Logger::logWithEnd(const std::string &str, const std::string &end) {
	const size_t size = size_t(out.tellp());
	const size_t maxSize = 100 << 10;//100 kB
	if (size >= maxSize) return;

	size_t lost = maxSize - size;
	if (lost >= str.size()) {
		lost = str.size();
	}else {
		lost = String::getCorrectCropIndex(str, lost);
	}
	out << std::string_view(str.c_str(), lost) << end;
	out.flush();
}
void Logger::logEvent(const std::string &event, double time, bool lastInGroup) {
	out << event << ": " << int(time * 1000) << " ms\n";
	if (lastInGroup) {
		out << "\n\n";
	}
	out.flush();
}
