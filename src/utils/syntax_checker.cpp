#include "syntax_checker.h"

#include <iostream>

std::map<String, std::vector<SyntaxPart>> SyntaxChecker::mapSyntax;

void SyntaxChecker::addBlockChildren(const String &parents, const String &childs, const bool clear) {
	const std::vector<String> parentBlocks = parents.split(", ");
	const std::vector<String> childBlocks = childs.split(", ");

	if (clear) {
		for (const String &parentBlock : parentBlocks) {
			if (!parentBlock) continue;
			mapSyntax[parentBlock].clear();
		}
	}

	for (const String &words : childBlocks) {
		if (!words) continue;

		std::vector<String> wordsVec = words.split(' ');
		for (String &s : wordsVec) {
			s.replaceAll("---", " ");
		}

		SyntaxPart sp;
		sp.name = wordsVec[0];
		sp.prevs.insert(sp.prevs.begin(), wordsVec.begin() + 1, wordsVec.end());
		sp.superParent = SuperParent::NONE;

		for (const String &parentBlock : parentBlocks) {
			if (!parentBlock) continue;

			for (char p : parentBlock) {
				if (p != ' ') {
					mapSyntax[parentBlock].push_back(sp);
					break;
				}
			}
		}
	}
}
void SyntaxChecker::setSuperParents(const String &nodesStr, const int superParent) {
	const std::vector<String> nodes = nodesStr.split(", ");

	for (auto &it : mapSyntax) {
		std::vector<SyntaxPart> &sps = it.second;

		for (SyntaxPart &sp : sps) {
			if (sp.name && std::find(nodes.begin(), nodes.end(), sp.name) != nodes.end()) {
				sp.superParent |= superParent;
			}
		}
	}
}

void SyntaxChecker::init() {
	const String screenElems = ", vbox, hbox, null, add, image, text, textbutton, button, imagebutton, ";

	const String windowProps = ", use, key, ";
	const String containerProps = ", has, spacing, xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, align, anchor, pos, xysize, ";
	const String textProps = ", color, size, font, size, text_align, ";

	const String transformProps = containerProps +
								  "repeat, linear, ease, easein, easeout, pause, rotate, zoom, alpha, crop, size, reset, ";

	const String conditions = ", if, elif if elif, else if elif for while, ";

	addBlockChildren("main", "init, init---python, label, screen");
	addBlockChildren("init", "for, while, " + conditions + "$, python, image");
	mapSyntax["init python"] = mapSyntax["menu"] = std::vector<SyntaxPart>();
	addBlockChildren("label, if, elif, else, for, while", "for, while" + conditions + "pause, $, python, image, menu, show, hide, scene, nvl, window, jump, call, play, stop, with");
	addBlockChildren("menu", "menuItem");

	mapSyntax["menuItem"] = mapSyntax["label"];
	SyntaxPart t;
	t.name = "pass";
	t.superParent = SuperParent::LABEL;
	mapSyntax["menuItem"].push_back(t);

	addBlockChildren("if, elif, else, for, while", screenElems + "pass, pause, $, python, image, menu, show, hide, scene, nvl, window, jump, call, play, stop, with, continue, break");
	addBlockChildren("for, while", "continue, break");

	addBlockChildren("show, scene, transform, window" + screenElems, transformProps);

	addBlockChildren("screen", "window");
	addBlockChildren("window, if, elif, else, for, while", windowProps);
	addBlockChildren("window" + screenElems, screenElems + "imagemap" + conditions + "for, while, pass");

	addBlockChildren("imagemap", "hotspot, ground, hover" + containerProps);
	addBlockChildren("hotspot", "action");

	addBlockChildren("window, vbox, hbox", containerProps);
	addBlockChildren("window, vbox, hbox, if, else, elif, for, while", "$, python");
	addBlockChildren("null", "width, height", true);
	addBlockChildren("text, textbutton", textProps);

	addBlockChildren("key, button, textbutton", "action");
	addBlockChildren("button, textbutton", "background, hover_background");

	setSuperParents("init, init python, label, screen", SuperParent::MAIN);
	setSuperParents("play, stop, show, hide, scene, nvl, window, with, jump, call, menu, menuItem", SuperParent::LABEL);
	setSuperParents("window" + screenElems + "imagemap, hotspot, ground, hover" + windowProps + containerProps + textProps +
					"spacing, action, background, hover_background, width, height", SuperParent::SCREEN);

	const int ALL = SuperParent::INIT | SuperParent::LABEL | SuperParent::SCREEN;
	setSuperParents(transformProps + "$, pass, break, continue, python, if, elif, else, for, while, image", ALL);

	//check
	if (!true) {
		for (auto it : mapSyntax) {
			for (const SyntaxPart &sp : it.second) {
				if (sp.superParent == SuperParent::NONE) {
					std::cout << "Неинициализированный узел (SyntaxPart) {parent: " + it.first + ", child: " + sp.name + "}" << '\n';
				}
			}
		}
	}
}

bool SyntaxChecker::check(const String &parent, const String &child, const String &prevChild, const int superParent, bool &thereIsNot) {
	thereIsNot = mapSyntax.find(parent) == mapSyntax.end();
	if (thereIsNot) {
		return superParent == SuperParent::LABEL;
	}
	const std::vector<SyntaxPart> sps = mapSyntax[parent];

	for (const SyntaxPart &sp : sps) {
		if (sp.name == child) {
			return sp.check(prevChild, superParent);
		}
	}
	thereIsNot = true;
	return superParent == SuperParent::LABEL;
}
