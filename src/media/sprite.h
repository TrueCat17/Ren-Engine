#ifndef SPRITE_H
#define SPRITE_H

#include <string>
#include <vector>

typedef struct _object PyObject;
class Node;

class Sprite {
public:
	static void registerImage(Node *imageNode);
	static bool imageWasRegistered(const std::string &name);
	static void clearImages();

	static PyObject* getImageDeclAt(const std::string &name);
	static const std::vector<std::string>& getChildrenImagesDeclAt(const std::string &name);
};

#endif // SPRITE_H
