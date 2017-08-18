#include "screen_continue.h"


ScreenContinue::ScreenContinue(Node *node): ScreenChild(node, nullptr) {

}

void ScreenContinue::calculateProps() {
	throw ContinueException();
}
