#ifndef TEXT_H
#define TEXT_H

#include "gui/screen/child.h"

class TextField;

struct TextParams {
	std::string font;

	Uint32 color = 0;
	Uint32 outlinecolor = 0;

	float halign;
	float valign;

	float size, size_min, size_max;
	bool size_is_float:1;
	bool size_min_is_float:1;
	bool size_max_is_float:1;

	bool set_font:1;
	bool set_color:1;
	bool set_outlinecolor:1;
	bool set_halign:1;
	bool set_valign:1;

	Uint8 font_style = 0;
	Uint8 set_font_style = 0;
};

class Text: public Child {
private:
	std::string prevText;

	const bool hasOutlineColor;
	const bool hasHoverOutlineColor;
public:

	bool prevParamsIsHover = false;
	bool curParamsIsHover = false;
	TextParams mainParams;
	TextParams hoverParams;

	TextField *tf;

	Text(Node *node, Screen *screen);
	virtual ~Text();

	virtual void updateRect(bool needUpdatePos = true);
	virtual void updateGlobal();
};

#endif // TEXT_H
