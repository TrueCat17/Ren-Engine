#include "text.h"

#include <tuple>

#include "gui/text_field.h"

#include "utils/math.h"
#include "utils/stage.h"

Text::Text(Node *node, Screen *screen):
    Child(node, nullptr, screen),
    hasOutlineColor(node->getProp("outlinecolor")),
    hasHoverOutlineColor(node->getProp("hover_outlinecolor")),
    tf(new TextField())
{
	addChildAt(tf, 0);
}
Text::~Text() {
	delete tf;
}

#define makeGetFunction(prop) \
static inline \
decltype(TextParams::prop) get_##prop(const Text *text) { \
	if (text->curParamsIsHover && text->hoverParams.set_##prop) { \
		return text->hoverParams.prop; \
	} \
	return text->mainParams.prop; \
}
makeGetFunction(font)
makeGetFunction(color)
makeGetFunction(outlinecolor)
makeGetFunction(halign)
makeGetFunction(valign)


#define makeGetSize(postfix) \
static inline \
float get_size##postfix(const Text *text) { \
	const TextParams *params = nullptr; \
	if (text->curParamsIsHover && text->hoverParams.size##postfix > 0) { \
		params = &text->hoverParams; \
	} else { \
		params = &text->mainParams; \
	} \
	\
	float res = params->size##postfix; \
	int k = params->size##postfix##_is_float ? Stage::height : 1; \
	return res * float(k); \
}
makeGetSize()
makeGetSize(_min)
makeGetSize(_max)

static inline
bool get_enable_outline(const Text *text, bool hasOutlineColor, bool hasHoverOutlineColor) {
	if (text->curParamsIsHover) {
		if (text->hoverParams.set_outlinecolor) {
			return true;
		}
		if (hasHoverOutlineColor || !hasOutlineColor) {
			return false;
		}
	}
	return text->mainParams.set_outlinecolor;
}

static inline
Uint8 get_font_style(const Text *text) {
	Uint8 res = text->mainParams.font_style;
	if (text->curParamsIsHover) {
		for (int i = 0; i < 8; ++i) {
			int mask = 1 << i;
			if (text->hoverParams.set_font_style & mask) {
				if (text->hoverParams.font_style & mask) {
					res |= mask;
				}else {
					res &= ~mask;
				}
			}
		}
	}
	return res;
}


void Text::updateRect(bool) {
	tf->enable = true;

	float width  = xsize * float(xsize_is_float ? Stage::width  : 1) * globalZoomX;
	float height = ysize * float(ysize_is_float ? Stage::height : 1) * globalZoomY;

	if (first_param.empty() && prevText.empty()) {
		setWidth(std::max<float>(width, 0));
		setHeight(std::max<float>(height, 0));
		return;
	}

	Uint32 color = get_color(this);
	Uint32 outlineColor = get_outlinecolor(this);
	bool enableOutline = get_enable_outline(this, hasOutlineColor, hasHoverOutlineColor);
	Uint8 fontStyle = get_font_style(this);

	auto curParams = std::tie(
	    first_param,
	    width,
	    height,
	    color,
	    outlineColor,
	    enableOutline,
	    fontStyle
	);
	auto prevParams = std::tie(
	    prevText,
	    tf->maxWidth,
	    tf->maxHeight,
	    tf->mainStyle.color,
	    tf->mainStyle.outlineColor,
	    tf->mainStyle.enableOutline,
	    tf->mainStyle.fontStyle
	);

	bool needUpdate = false;
	if (prevParams != curParams) {
		prevParams = curParams;
		needUpdate = true;
	}

	std::string font = get_font(this);

	float size = get_size(this);
	float min = get_size_min(this);
	float max = get_size_max(this);
	if (min > 0 && size < min) size = min;
	if (max > 0 && size > max) size = max;
	size *= globalZoomY;

	if (std::tuple(font, size) != std::tuple(tf->mainStyle.fontName, tf->mainStyle.fontSize)) {
		tf->setFont(font, size);
		needUpdate = true;
	}

	float halign = get_halign(this);
	float valign = get_valign(this);
	if (needUpdate || std::tuple(halign, valign) != std::tuple(tf->getHAlign(), tf->getVAlign())) {
		tf->setText(first_param);

		if (width  <= 0) width  = tf->getWidth();
		if (height <= 0) height = tf->getHeight();
		setWidth(width);
		setHeight(height);
		tf->setWidth(width);
		tf->setHeight(height);

		tf->setAlign(halign, valign);
	}

	updatePos();
}

void Text::updateGlobal() {
	float prevGlobalRotate = getGlobalRotate();
	Child::updateGlobal();

	if (!prevText.empty() && !Math::floatsAreEq(prevGlobalRotate, getGlobalRotate())) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}
