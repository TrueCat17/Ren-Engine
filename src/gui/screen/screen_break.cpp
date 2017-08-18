#include "screen_break.h"

ScreenBreak::ScreenBreak(Node *node): ScreenChild(node, nullptr) {

}

void ScreenBreak::calculateProps() {
	throw BreakException();
}
