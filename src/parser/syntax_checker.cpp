#include "syntax_checker.h"

#include <map>
#include <set>

#include "parser/parser.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::set<std::string> knownScreenLeafs;
bool SyntaxChecker::isKnownScreenLeaf(const std::string &name) {
	return knownScreenLeafs.find(name) != knownScreenLeafs.end();
}

struct SyntaxPart {
	std::vector<std::string> prevs;
	int superParent = SuperParent::NONE;

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
		Utils::outMsg("SyntaxChecker::getScreenProps", "Type <" + type + "> not found");
		static std::vector<std::string> res;
		return res;
	}

	std::map<std::string, SyntaxPart> children = syntaxIt->second;
	std::set<std::string> tmp;

	for (const auto &[name, _] : children) {
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


static std::string getXY(const std::string &props) {
	std::string res;
	std::vector<std::string> propsVec = String::split(props, ", ");
	for (const std::string &prop : propsVec) {
		res += ", x" + prop;
		res += ", y" + prop;
		res += ", " + prop;
	}
	return res;
}

void SyntaxChecker::init() {
	const std::string screenElems = ", vbox, hbox, null, image, text, textbutton, button, use, ";

	const std::string screenProps = ", key, has, modal, ignore_modal, save, zorder, ";
	const std::string spacingProps = ", spacing, spacing_min, spacing_max, ";
	const std::string simpleProps = getXY("align, anchor, pos, size, size_min, size_max, zoom") +
	                                ", crop, rotate, alpha, clipping, skip_mouse, style, ";
	const std::string textProps =
	        ", text_size, text_size_min, text_size_max"
	        ", color, outlinecolor, font, text_align, text_valign, bold, italic, underline, strikethrough, ";
	const std::string buttonProps =
	        ", action, alternate, hovered, unhovered, activate_sound, hover_sound, mouse, selected, ";

	const std::string extraScreenLeafs = "pass, first_param, $, python, break, continue, ground, hover, corner_sizes";
	auto propsList = { &screenProps, &spacingProps, &simpleProps, &textProps, &buttonProps, &extraScreenLeafs };
	for (const std::string *props : propsList) {
		std::vector<std::string> vecProps = String::split(*props, ", ");
		for (const std::string &prop : vecProps) {
			if (!prop.empty()) {
				knownScreenLeafs.insert(prop);
			}
		}
	}

	const std::string conditions = ", if, elif if elif, else if elif for while, ";

	addBlockChildren("main", "init, init---python, label, screen, translate, translate---strings");
	addBlockChildren("init", "for, while, " + conditions + "$, python, image, style");
	mapSyntax["init python"] = {};
	addBlockChildren("label, if, elif, else, for, while",
		"pass, return, for, while" + conditions +
		"pause, $, python, image, menu, show, hide, scene, nvl, window, jump, call, play, stop, with");

	addBlockChildren("menu", "menuItem");
	mapSyntax["menuItem"] = mapSyntax["label"];

	addBlockChildren("if, elif, else, for, while", screenElems + "imagemap, hotspot, " + "continue, break");

	addBlockChildren("screen, vbox, hbox", spacingProps);
	addBlockChildren("screen", screenProps);
	addBlockChildren("if, elif, else, for, while", "key");
	addBlockChildren("screen" + screenElems,
	                 screenElems + "imagemap" + conditions + "for, while, pass, $, python, key" + simpleProps);
	mapSyntax.erase(mapSyntax.find("use"));

	addBlockChildren("text, textbutton", textProps);

	std::vector<std::string> vecTextProps = String::split(textProps, ", ");
	for (auto &prop : vecTextProps) {
		if (!prop.empty()) {
			prop.insert(0, "hover_");
			knownScreenLeafs.insert(prop);
			addBlockChildren("textbutton", prop);
			setSuperParents(prop, SuperParent::SCREEN);
		}
	}

	addBlockChildren("key", "first_delay, delay, action");

	addBlockChildren("hotspot, button, textbutton", buttonProps);
	addBlockChildren("imagemap, button, textbutton", "ground, hover");
	addBlockChildren("imagemap", "hotspot, for, while" + conditions + simpleProps);

	addBlockChildren("imagemap, button, textbutton, image", "corner_sizes");
	setSuperParents("corner_sizes", SuperParent::SCREEN);

	setSuperParents("init, init python, translate, translate strings, label, screen", SuperParent::MAIN);
	setSuperParents("style", SuperParent::INIT);

	setSuperParents("return, play, stop, show, hide, scene, nvl, window, with, jump, call, menu, menuItem, pause", SuperParent::LABEL);
	setSuperParents(screenElems + "imagemap, hotspot, for, ground, hover" +
	                screenProps + spacingProps + simpleProps + textProps + buttonProps +
	                "action, first_delay, delay", SuperParent::SCREEN);

	addBlockChildren("translate strings", "old, new old");
	setSuperParents("old, new", SuperParent::TL_STRS);

	const int ALL = SuperParent::INIT | SuperParent::LABEL | SuperParent::SCREEN;
	setSuperParents("$, pass, break, continue, python, if, elif, else, while, image", ALL);
}

static const std::set<std::string> blocksWithAny({"scene", "show", "image", "contains", "block", "parallel", "style"});
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
