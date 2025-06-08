#ifndef SCREEN_UPDATE_FUNCS_H
#define SCREEN_UPDATE_FUNCS_H

#include "parser/node.h"

class Child;
using ScreenUpdateFunc = void(*)(Child *obj, size_t propIndex);


class ScreenUpdateFuncs {
public:
	static const std::vector<ScreenUpdateFunc>* getFuncs(const Node *node);
	static ScreenUpdateFunc getFunc(const std::string &type, std::string propName);

	static void initNodeFuncs(Node *node);
	static void clear();
};

#endif // SCREEN_UPDATE_FUNCS_H
