#include "text.h"

#include <tuple>

#include "../text_field.h"

#include "utils/math.h"
#include "utils/stage.h"

Text::Text(Node *node, Screen *screen):
    Container(node, this, screen),
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
bool get_enable_outline(const Text *text) {
	if (text->curParamsIsHover && text->hoverParams.set_outlinecolor) {
		return true;
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


void Text::updateSize() {
	Child::updateSize();
	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updateSize();
		}
	}

	float width = xsize * float(xsize_is_float ? Stage::width : 1);
	float height = ysize * float(ysize_is_float ? Stage::height : 1);

	if ((first_param.empty() && prevText.empty()) ||
	    (globalZoomX <= 0 || globalZoomY <= 0))
	{
		setWidthWithMinMax(std::max<float>(width, 0));
		setHeightWithMinMax(std::max<float>(height, 0));
		tf->enable = false;
		return;
	}

	Uint32 color = get_color(this);
	Uint32 outlineColor = get_outlinecolor(this);
	bool enableOutline = get_enable_outline(this);
	Uint8 fontStyle = get_font_style(this);

	int zoomedWidth  = int(width  * globalZoomX);
	int zoomedHeight = int(height * globalZoomY);

	auto curParams = std::tie(
	    first_param,
	    zoomedWidth,
	    zoomedHeight,
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

	const std::string *fontName = get_font(this);
	if (!fontName) {
		printf("fontName == nullptr\n");
		std::abort();
	}

	float size = get_size(this);
	float min = get_size_min(this);
	float max = get_size_max(this);
	if (min > 0 && size < min) size = min;
	if (max > 0 && size > max) size = max;
	size = std::floor(size * globalZoomY);

	if (fontName != tf->mainStyle.fontName ||
	    !Math::floatsAreEq(size, tf->mainStyle.fontSize))
	{
		tf->setFont(fontName, size);
		needUpdate = true;
	}

	if (needUpdate) {
		tf->setText(first_param);
	}

	if (width <= 0) {
		setWidthWithMinMax(tf->getWidth() / globalZoomX);
	}
	if (height <= 0) {
		setHeightWithMinMax(tf->getHeight() / globalZoomY);
	}

	float halign = get_halign(this);
	float valign = get_valign(this);
	if (!needUpdate && (
	    !Math::floatsAreEq(halign, tf->getHAlign()) ||
	    !Math::floatsAreEq(valign, tf->getVAlign())
	)) {
		needUpdate = true;
	}
	if (needUpdate) {
		tf->setAlign(halign, valign);
	}
}

void Text::updateGlobal() {
	float prevGlobalRotate = getGlobalRotate();
	Container::updateGlobal();

	if (!prevText.empty() && !Math::floatsAreEq(prevGlobalRotate, getGlobalRotate())) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}

void Text::updateTexture() {
	tf->enable = true;
	Container::updateTexture();
}
