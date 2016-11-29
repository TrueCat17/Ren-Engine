#include "node.h"

#include <algorithm>

#include "gv.h"
#include "config.h"

#include "gui/gui.h"
#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/music_channel.h"

#include "utils/game.h"
#include "utils/utils.h"

std::vector<Node*> Node::nodes;
bool Node::jumped = false;

void Node::execute() {
	if (jumped) return;

	if (command == "main") {
		for (size_t i = 0; i < children.size(); ++i) {
			Node *node = children[i];
			if (node->command == "screen") {
				Screen::declare(node);
			}
		}

		std::vector<Node*> initBlocks;
		for (size_t i = 0; i < children.size(); ++i) {
			Node *node = children[i];
			if (node->command == "init" || node->command == "init python") {
				initBlocks.push_back(node);
			}
		}
		std::sort(initBlocks.begin(), initBlocks.end(), [](Node* a, Node* b) { return a->priority < b->priority; });

		for (Node *i : initBlocks) {
			i->execute();
		}

		mainLabel = Utils::execPython("mods_last_key", true);
		if (mainLabel.find("_menu") == size_t(-1)) {
			Screen::show("dialogue_box", false);
		}
		jump(mainLabel);

		if (!GV::exit && Utils::execPython("cur_location_name", true) == "None") {
			GV::updateGuard.lock();
			GV::renderGuard.lock();
			{
				GV::inGame = false;

				if (GV::screens) {
					GV::screens->clearChildren();
				}
				Screen::clear();

				Utils::destroyAllTextures();
				Utils::destroyAllSurfaces();
			}
			GV::renderGuard.unlock();
			GV::updateGuard.unlock();

			MusicChannel::clear();
			Style::destroyAll();

			if (!Game::modeStarting) {
				Game::startMod("main_menu");
			}
		}
	}else

	if (command == "init" || command == "label") {
		for (size_t i = 0; GV::inGame && i < children.size(); ++i) {
			Node *node = children[i];

			for (size_t j = 1;
				  j < size_t(Config::get("count_image_commands_to_processing_beforehand").toInt())
				  &&
				  i + j < children.size();
				 ++j)
			{
				Node *node = children[i + j];
				if ((node->command == "show" && !node->params.startsWith("screen ")) || node->command == "scene") {
					//Sprite::loadResources(node->params);
				}
			}

			node->execute();

			while (GV::inGame && Utils::execPython("waiting_moving", true) == "True") {
				Utils::sleep(1);
			}
		}
	}else

	if (command == "show") {
		if (params.startsWith("screen ")) {
			String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::show(screenName, false);
		}else {
			//Sprite::show(params);
		}
	}else

	if (command == "hide") {
		if (params.startsWith("screen ")) {
			String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::hide(screenName, false);
		}else {
			//Sprite::hide(params);
		}
	}else

	if (command == "scene") {
		Utils::execPython("set_scene('" + params + "')");
	}else

	if (command == "with") {
		Utils::sleep(100);
	}else

	if (command == "window") {
		String code = "window_" + params + "()";
		Utils::execPython(code);
	}else

	if (command == "text") {
		//return;

		size_t i = params.find_first_not_of(' ');
		params.erase(0, i);

		size_t startText = 0;
		String nick;

		if (params[0] != '"') {//"имя" указано?
			i = params.find(' ');
			nick = params.substr(0, i);

			startText = i + 1;
		}
		size_t endText = params.size();
		String text = params.substr(startText, endText - startText);

		if (!nick) {
			nick = "narrator";
		}

		String code = nick + "(" + text + ")";
		Utils::execPython(code);
	}else

	if (command == "python" || command == "init python") {
		Utils::execPython(params);
	}else

	if (command == "pause") {
		String strTime = Utils::execPython(params, true);
		int time = strTime.toDouble() * 1000;
		Utils::sleep(time);
	}else

	if (command == "jump") {
		jump(params);
	}else

	if (command == "play") {
		MusicChannel::play(params);
	}else

	if (command == "stop") {
		MusicChannel::stop(params);
	}else

	if (command == "image") {
		Utils::registerImage(params);
	}else

	if (command == "nvl") {
		String code = "nvl_" + params + "()";
		Utils::execPython(code);
	}else


	if (command == "menu") {/*
		ChooseMenu::make(this);

		Node *chooseNode = ChooseMenu::chooseNode;
		for (size_t i = 0; GV::inGame && i < chooseNode->children.size(); ++i) {
			Node *childNode = chooseNode->children[i];
			childNode->execute();
		}*/
	}else

	if (command == "if") {
		String execRes = Utils::execPython(params, true);
		condIsTrue = execRes == "True";

		if (condIsTrue) {
			for (size_t i = 0; GV::inGame && i < children.size(); ++i) {
				Node *node = children[i];
				node->execute();
			}
		}
	}else

	if (command == "elif") {
		Node *t = prevNode;
		while (t->command != "if" && !t->condIsTrue) {
			t = t->prevNode;
		}
		if (t->condIsTrue) return;

		String execRes = Utils::execPython(params, true);
		condIsTrue = execRes == "True";

		if (condIsTrue) {
			for (size_t i = 0; GV::inGame && i < children.size(); ++i) {
				Node *node = children[i];
				node->execute();
			}
		}
	}else

	if (command == "else") {
		Node *t = prevNode;
		while (t->command != "if" && !t->condIsTrue) {
			t = t->prevNode;
		}
		if (t->condIsTrue) return;

		for (size_t i = 0; GV::inGame && i < children.size(); ++i) {
			Node *node = children[i];
			node->execute();
		}
	}else

	if (command == "while") {
		String cond = "bool(" + params + ")";
		while (GV::inGame && Utils::execPython(cond, true) == "True") {
			for (size_t i = 0; GV::inGame && i < children.size(); ++i) {
				Node* node = children[i];
				node->execute();
			}
		}
	}else

	{

		if (command != "pass") {
			Utils::outMsg("Node::execute: Неизвестный тип команды <" + command + ">");
		}
	}

	int toSleep = Game::getFrameTime();
	while (GV::inGame && Utils::execPython("read", true) != "True") {
		Utils::sleep(toSleep);
	}
}

void Node::jump(String label) {
	for (size_t i = 0; i < GV::mainExecNode->children.size(); ++i) {
		Node *node = GV::mainExecNode->children[i];
		if (node->command == "label" && node->name == label) {
			node->execute();
			jumped = true;
			return;
		}
	}
	Utils::outMsg("Метка <" + label + "> не найдена");
}

String Node::getProp(const String& name) const {
	String res;

	if (props.find(name) != props.end()) {
		String toExec = props.at(name);
		res = Utils::execPython(toExec, true);
	}

	if (!res) {
		static std::vector<String> exceptions = String("screen if elif for while else hotspot").split(' ');
		if (!Utils::in(command, exceptions)) {
			res = Style::getProp(command, name);//command is type of this screenChild
		}
	}

	return res;
}
String Node::getPropCode(const String& name) const {
	if (props.find(name) == props.end()) return "";

	String res = props.at(name);
	return res;
}


void Node::destroyAll() {
	for (size_t i = 0; i < nodes.size(); ++i) {
		Node *node = nodes[i];
		delete node;
	}
	nodes.clear();
}
