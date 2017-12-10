#ifndef SCREEN_PROP_H
#define SCREEN_PROP_H

#include <stddef.h>


enum ScreenProp {
	//common
	X_POS,
	Y_POS,
	X_ANCHOR,
	Y_ANCHOR,
	X_ALIGN,
	Y_ALIGN,
	X_SIZE,
	Y_SIZE,
	CROP,
	ROTATE,
	ALPHA,

	//image, buttons, imagemap
	IMAGE_PATH,
	GROUND,
	HOVER,

	//hotspot
	RECTANGLE,

	//buttons
	MOUSE,

	//screen, vbox, hbox
	SPACING,
	MODAL,
	ZORDER,

	//key
	KEY,
	DELAY,
	FIRST_DELAY,

	//text
	TEXT,
	FONT,
	TEXT_SIZE,
	TEXT_ALIGN,
	TEXT_VALIGN,
	COLOR,


	//not use, must be ALWAYS last
	LAST_POINT
};
constexpr size_t COUNT_PROPS = ScreenProp::LAST_POINT;

extern const char* screenPropNames[COUNT_PROPS + 1];


#endif // SCREEN_PROP_H