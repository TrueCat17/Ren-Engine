#ifndef SCREEN_H
#define SCREEN_H

#include <vector>

#include "gui/screen/screen_container.h"

class Screen: public ScreenContainer {
private:
	static std::vector<Node*> declared;
	static Node* getDeclared(const String& name);

	static std::vector<Screen*> created;
	static Screen* getCreated(const String& name);

	size_t countAsMain = 0;
	size_t countAsUsed = 0;

	void addShowedCount(bool depended);
	bool subShowedCount(bool depended);

	Screen(Node *node, bool depended);
	virtual ~Screen();

public:
	static void declare(Node *node);
	static void clear();

	static Screen* show(const String& name, bool depended);
	static void hide(const String& name, bool depended);


	String name;
};

#endif // SCREEN_H
