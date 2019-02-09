#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>

#include "utils/string.h"


class Scenario {
public:
	static std::vector<std::pair<String, String>> getStackToSave();

	static void execute();
	static void jumpNext(const std::string &label, bool isCall);
};

#endif // SCENARIO_H
