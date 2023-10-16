#ifndef SCREEN_CODE_GENERATOR_H
#define SCREEN_CODE_GENERATOR_H

#include "parser/node.h"

class ScreenCodeGenerator {
public:
	static std::string get(Node *screenNode);
	static void clear();
};

#endif // SCREEN_CODE_GENERATOR_H
