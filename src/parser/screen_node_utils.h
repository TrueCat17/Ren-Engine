#ifndef SCREEN_NODE_UTILS_H
#define SCREEN_NODE_UTILS_H

#include <map>

#include "parser/node.h"
#include "utils/string.h"

class Child;
typedef void(*ScreenUpdateFunc)(Child *obj, size_t propIndex);


class ScreenNodeUtils {
public:
	static void init(Node *node);

	static String getScreenCode(const Node *screenNode);

	static const std::vector<ScreenUpdateFunc> *getUpdateFuncs(const Node *node);
	static ScreenUpdateFunc getUpdateFunc(const String &type, String propName);

	static void clear();
};

#endif // SCREEN_NODE_UTILS_H
