#include "screen_break.h"

#include "utils/utils.h"

ScreenBreak::ScreenBreak(Node *node):
	ScreenChild(node, nullptr, nullptr)
{ }

void ScreenBreak::calculateProps() {
	throw BreakException();
}
