#include "screen_window.h"


ScreenWindow::ScreenWindow(Node *node): ScreenContainer(node, this) {

}

int ScreenWindow::getMinX() const {
	return DisplayObject::getMinX();
}
int ScreenWindow::getMinY() const {
	return DisplayObject::getMinY();
}
int ScreenWindow::getMaxX() const {
	return DisplayObject::getMaxX();
}
int ScreenWindow::getMaxY() const {
	return DisplayObject::getMaxY();
}
