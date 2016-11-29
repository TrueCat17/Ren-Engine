#ifndef PART_H
#define PART_H

#include <vector>

#include <utils/string.h>


class Part {
public:
	String type;

	std::vector<String> parts;

	Part();

	void execute();
};

#endif // PART_H
