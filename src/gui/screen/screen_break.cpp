#include "screen_break.h"

#include "utils/utils.h"

ScreenBreak::ScreenBreak(Node *node): ScreenChild(node, nullptr) {

}

void ScreenBreak::updateProps() {
	throw BreakException();
}
