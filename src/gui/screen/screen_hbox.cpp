#include "screen_hbox.h"

ScreenHBox::ScreenHBox(Node *node, Screen *screen):
	ScreenContainer(node, this, screen)
{
	hasHBox = true;
}
