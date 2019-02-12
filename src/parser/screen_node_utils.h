#ifndef SCREEN_NODE_UTILS_H
#define SCREEN_NODE_UTILS_H

#include "parser/node.h"

class Child;
typedef void(*ScreenUpdateFunc)(Child *obj, size_t propIndex);


class ScreenNodeUtils {
public:
	static void init(Node *node);

	static std::string getScreenCode(const Node *screenNode);

	static const std::vector<ScreenUpdateFunc> *getUpdateFuncs(const Node *node);
	static ScreenUpdateFunc getUpdateFunc(const std::string &type, std::string propName);

	static void clear();
};

#endif // SCREEN_NODE_UTILS_H
