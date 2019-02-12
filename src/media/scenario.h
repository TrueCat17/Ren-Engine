#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <string>


class Scenario {
public:
	static std::vector<std::pair<std::string, std::string>> getStackToSave();

	static void execute();
	static void jumpNext(const std::string &label, bool isCall);
};

#endif // SCENARIO_H
