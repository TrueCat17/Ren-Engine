#include "syntax_checker.h"

#include <map>
#include <set>

#include "parser/parser.h"
#include "utils/string.h"
#include "utils/utils.h"


struct SyntaxPart {
	std::vector<std::string> prevs;
	int superParent;

	inline bool check(const std::string &prevChild, const int superParent) const {
		if (!(this->superParent & superParent)) return false;

		if (prevs.empty()) return true;

		for (const std::string &prev : prevs) {
			if (prev == prevChild) return true;
		}
		return false;
	}
};

static std::map<std::string, std::map<std::string, SyntaxPart>> mapSyntax;


static void addBlockChildren(const std::string &parents, const std::string &children) {
	const std::vector<std::string> parentBlocks = String::split(parents, ", ");
	const std::vector<std::string> childBlocks = String::split(children, ", ");

	for (const std::string &words : childBlocks) {
		if (words.empty()) continue;

		std::vector<std::string> wordsVec = String::split(words, " ");
		for (std::string &s : wordsVec) {
			String::replaceAll(s, "---", " ");
		}

		const std::string &name = wordsVec[0];

		SyntaxPart sp;
		sp.prevs.insert(sp.prevs.begin(), wordsVec.begin() + 1, wordsVec.end());
		sp.superParent = SuperParent::NONE;

		for (const std::string &parentBlock : parentBlocks) {
			if (parentBlock.empty()) continue;
			mapSyntax[parentBlock][name] = sp;
		}
	}
}
static void setSuperParents(const std::string &nodesStr, const int superParent) {
	const std::vector<std::string> nodes = String::split(nodesStr, ", ");

	for (const std::string &node : nodes) {
		for (auto &p : mapSyntax) {
			std::map<std::string, SyntaxPart> &sps = p.second;

			auto it = sps.find(node);
			if (it != sps.end()) {
				it->second.superParent |= superParent;
			}
		}
	}
}


static std::map<std::string, std::vector<std::string>> mapProps;
const std::vector<std::string> &SyntaxChecker::getScreenProps(const std::string &type) {
	auto it = mapProps.find(type);
	if (it != mapProps.end()) return it->second;

	auto syntaxIt = mapSyntax.find(type);
	if (syntaxIt == mapSyntax.end()) {
		Utils::outMsg("SyntaxChecker::getProps", "Type <" + type + "> not found");
		static std::vector<std::string> res;
		return res;
	}

	std::map<std::string, SyntaxPart> children = syntaxIt->second;
	std::set<std::string> tmp;

	for (auto &[name, _] : children) {
		bool isFakeComp, isProp, isEvent;
		Parser::getIsFakeOrIsProp(name, isFakeComp, isProp, isEvent);

		if (!isProp || isEvent ||
			name == "has" || name == "pass" || name == "style" ||
			name == "align" || name == "xalign" || name == "yalign"
		) continue;

		tmp.insert(name);
	}

	std::vector<std::string> res;
	for (const std::string &name : tmp) {
		if (!tmp.count('x' + name)) {//not add <name> if there is <xname> (for example pos:xpos)
			res.push_back(name);
		}
	}

	return mapProps[type] = res;
}


void SyntaxChecker::init() {
	const std::string screenElems = ", vbox, hbox, null, image, text, textbutton, button, use, ";

	const std::string screenProps = ", key, has, modal, zorder, ";
	const std::string simpleProps = ", style, xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, align, anchor, pos, size, crop, rotate, alpha, clipping, ";
	const std::string textProps = ", color, outlinecolor, font, text_size, text_align, text_valign, bold, italic, underline, strikethrough, ";
	const std::string buttonProps = ", alternate, hovered, unhovered, activate_sound, hover_sound, mouse, ";

	const std::string conditions = ", if, elif if elif, else if elif for while, ";

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

static const std::set<std::string> blocksWithAny({"scene", "show", "image", "contains", "block", "parallel"});
bool SyntaxChecker::check(const std::string &parent, const std::string &child, const std::string &prevChild, const int superParent, bool &isText) {

	if ((superParent == SuperParent::INIT || superParent == SuperParent::LABEL) &&
		blocksWithAny.count(parent))
	{
		isText = false;
		return true;
	}

	auto it = mapSyntax.find(parent);
	isText = it == mapSyntax.end();
	if (isText) return false;

	const std::map<std::string, SyntaxPart> &sps = it->second;

	auto it2 = sps.find(child);
	if (it2 != sps.end()) {
		const SyntaxPart &sp = it2->second;
		return sp.check(prevChild, superParent);
	}

	if (superParent == SuperParent::LABEL) {
		isText = child != "continue" && child != "break";
		return true;
	}

	isText = true;
	return false;
}
