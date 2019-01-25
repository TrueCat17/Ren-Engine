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

	static String getScreenCode(Node *screenNode);

	static const std::vector<ScreenUpdateFunc>& getUpdateFuncs(Node *node);
	static ScreenUpdateFunc getUpdateFunc(String type, String propName);

	static void clear();
};

#endif // SCREEN_NODE_UTILS_H
