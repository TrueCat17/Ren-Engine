#include "screen_continue.h"

#include "utils/utils.h"

ScreenContinue::ScreenContinue(Node *node): ScreenChild(node, nullptr) {

}

void ScreenContinue::updateProps() {
	throw ContinueException();
}
