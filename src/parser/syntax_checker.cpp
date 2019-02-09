#include "syntax_checker.h"

#include <set>

#include "parser/parser.h"
#include "utils/utils.h"


struct SyntaxPart {
	std::vector<String> prevs;
	int superParent;

	inline bool check(const String &prevChild, const int superParent) const {
		if (!(this->superParent & superParent)) return false;

		if (prevs.empty()) return true;

		for (const String &prev : prevs) {
			if (prev == prevChild) return true;
		}
		return false;
	}
};

static std::map<String, std::map<String, SyntaxPart>> mapSyntax;


static void addBlockChildren(const String &parents, const String &childs) {
	const std::vector<String> parentBlocks = parents.split(", ");
	const std::vector<String> childBlocks = childs.split(", ");

	for (const String &words : childBlocks) {
		if (!words) continue;

		std::vector<String> wordsVec = words.split(' ');
		for (String &s : wordsVec) {
			s.replaceAll("---", " ");
		}

		const String &name = wordsVec[0];

		SyntaxPart sp;
		sp.prevs.insert(sp.prevs.begin(), wordsVec.begin() + 1, wordsVec.end());
		sp.superParent = SuperParent::NONE;

		for (const String &parentBlock : parentBlocks) {
			if (!parentBlock) continue;
			mapSyntax[parentBlock][name] = sp;
		}
	}
}
static void setSuperParents(const String &nodesStr, const int superParent) {
	const std::vector<String> nodes = nodesStr.split(", ");

	for (const String &node : nodes) {
		for (auto &p : mapSyntax) {
			std::map<String, SyntaxPart> &sps = p.second;

			auto it = sps.find(node);
			if (it != sps.end()) {
				it->second.superParent |= superParent;
			}
		}
	}
}


static std::map<String, std::vector<String>> mapProps;
const std::vector<String>& SyntaxChecker::getScreenProps(const String &type) {
	auto it = mapProps.find(type);
	if (it != mapProps.end()) return it->second;

	auto syntaxIt = mapSyntax.find(type);
	if (syntaxIt == mapSyntax.end()) {
		Utils::outMsg("SyntaxChecker::getProps", "Type <" + type + "> not found");
		static std::vector<String> res;
		return res;
	}

	std::map<String, SyntaxPart> children = syntaxIt->second;
	std::set<String> tmp;

	for (auto &[name, _] : children) {
		bool isFakeComp, isProp, isEvent;
		Parser::getIsFakeOrIsProp(name, isFakeComp, isProp, isEvent);

		if (!isProp || isEvent ||
			name == "has" || name == "pass" || name == "style" ||
			name == "align" || name == "xalign" || name == "yalign"
		) continue;

		tmp.insert(name);
	}

	std::vector<String> res;
	for (const String &name : tmp) {
		if (!tmp.count('x' + name)) {//not add <name> if there is <xname> (for example pos:xpos)
			res.push_back(name);
		}
	}

	return mapProps[type] = res;
}


void SyntaxChecker::init() {
	const String screenElems = ", vbox, hbox, null, image, text, textbutton, button, ";

	const String screenProps = ", use, key, has, modal, zorder, ";
	const String simpleProps = ", style, xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, align, anchor, pos, size, crop, rotate, alpha, ";
	const String textProps = ", color, font, text_size, text_align, text_valign, ";
	const String buttonProps = ", alternate, hovered, unhovered, activate_sound, hover_sound, mouse, ";

	const String conditions = ", if, elif if elif, else if elif for while, ";

	addBlockChildren("main", "init, init---python, label, screen");
	addBlockChildren("init", "for, while, " + conditions + "$, python, image");
	mapSyntax["init python"] = {};
	addBlockChildren("label, if, elif, else, for, while",
		"pass, return, for, while" + conditions +
		"pause, $, python, image, menu, show, hide, scene, nvl, window, jump, call, play, stop, with");

	addBlockChildren("menu", "menuItem");
	mapSyntax["menuItem"] = mapSyntax["label"];

	addBlockChildren("if, elif, else, for, while", screenElems + "imagemap, hotspot, " + "continue, break");

	addBlockChildren("screen, vbox, hbox", "spacing");
	addBlockChildren("screen, if, elif, else, for, while", screenProps);
	addBlockChildren("screen, vbox, hbox, null, image", screenElems + "imagemap" + conditions + "for, while, pass, $, python");
	addBlockChildren("screen" + screenElems, simpleProps);

	addBlockChildren("text, textbutton", textProps);

	addBlockChildren("key", "first_delay, delay");
	addBlockChildren("key, hotspot, button, textbutton", "action");

	addBlockChildren("hotspot, button, textbutton", buttonProps);
	addBlockChildren("imagemap, button, textbutton", "ground, hover");
	addBlockChildren("imagemap", "hotspot, for, while" + conditions + simpleProps);

	setSuperParents("init, init python, label, screen", SuperParent::MAIN);
	setSuperParents("return, play, stop, show, hide, scene, nvl, window, with, jump, call, menu, menuItem, pause", SuperParent::LABEL);
	setSuperParents(screenElems + "imagemap, hotspot, for, ground, hover" +
					screenProps + simpleProps + textProps + buttonProps +
					"spacing, action, first_delay, delay", SuperParent::SCREEN);

	const int ALL = SuperParent::INIT | SuperParent::LABEL | SuperParent::SCREEN;
	setSuperParents("$, pass, break, continue, python, if, elif, else, while, image", ALL);
}

static const std::set<String> blocksWithAny({"scene", "show", "image", "contains", "block", "parallel"});
bool SyntaxChecker::check(const String &parent, const String &child, const String &prevChild, const int superParent, bool &thereIsNot) {

	if ((superParent == SuperParent::INIT || superParent == SuperParent::LABEL) &&
		blocksWithAny.count(parent))
	{
		thereIsNot = false;
		return true;
	}

	auto it = mapSyntax.find(parent);
	thereIsNot = it == mapSyntax.end();
	if (thereIsNot) {
		return false;
	}
	const std::map<String, SyntaxPart> &sps = it->second;

	auto it2 = sps.find(child);
	if (it2 != sps.end()) {
		const SyntaxPart &sp = it2->second;
		return sp.check(prevChild, superParent);
	}

	thereIsNot = true;
	return superParent == SuperParent::LABEL;
}
