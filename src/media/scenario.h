#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <string>


class Scenario {
public:
	static bool initing;


	static std::vector<std::pair<std::string, std::string>> getStackToSave();

	static void execute(const std::string &loadPath);
	static void jumpNext(const std::string &label, bool isCall, const std::string &fileName, size_t numLine);
};

#endif // SCENARIO_H
