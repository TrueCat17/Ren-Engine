#include "screen_vbox.h"

ScreenVBox::ScreenVBox(Node *node, Screen *screen):
	ScreenContainer(node, this, screen)
{
	hasVBox = true;
}
