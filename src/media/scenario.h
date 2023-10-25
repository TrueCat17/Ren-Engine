#ifndef SCENARIO_H
#define SCENARIO_H

#include <inttypes.h>
#include <string>

class Scenario {
public:
	static bool initing;


	static void saveStack(const std::string &path);

	static void execute(const std::string &loadPath);
	static void jumpNext(const std::string &label, bool isCall, const std::string &fileName, uint32_t numLine);
};

#endif // SCENARIO_H
