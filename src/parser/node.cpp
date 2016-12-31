#include "node.h"

#include <algorithm>

#include "gv.h"
#include "config.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/music_channel.h"

#include "utils/game.h"
#include "utils/utils.h"

std::vector<Node*> Node::nodes;
bool Node::jumped = false;
bool Node::initing = false;

void Node::execute() {
	if (jumped || !GV::inGame) return;

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

		try {
			GV::initGuard.lock();
			initing = true;
			for (Node *i : initBlocks) {
				i->execute();
			}
			initing = false;
			GV::initGuard.unlock();

			mainLabel = Utils::execPython("mods_last_key", true);
			if (mainLabel.find("_menu") == size_t(-1)) {
				Screen::addToShowSimply("location");
				Screen::addToShowSimply("sprites");
				Screen::addToShowSimply("dialogue_box");
			}

			jump(mainLabel);
		}catch (ContinueException) {
			Utils::outMsg("Node::execute", "continue вне цикла");
		}catch (BreakException) {
			Utils::outMsg("Node::execute", "break вне цикла");
		}catch (StopException) {
			Utils::outMsg("Node::execute", "Неожидаемое исключение StopException (конец итератора)");
		}

		if (!Game::modeStarting && !GV::exit && Utils::execPython("cur_location_name", true) == "None") {
			Game::startMod("main_menu");
		}
	}else

	if (command == "init" || command == "label") {
		for (size_t i = 0; GV::inGame && i < children.size(); ++i) {
			Node *node = children[i];
			node->execute();

			while (!initing && Utils::execPython("character_moving", true) == "True") {
				Utils::sleep(1);
			}
		}
	}else

	if (command == "show") {
		if (params.startsWith("screen ")) {
			String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::addToShowSimply(screenName);
		}else {
			const std::vector<String> args = Utils::getArgs(params);
			if (args.empty()) {
				Utils::outMsg("Строка <show " + params + "> некорректна");
				return;
			}

			String argsStr = "'" + args[0] + "'";
			for (size_t i = 1; i < args.size(); ++i) {
				argsStr += ", '" + args[i] + "'";
			}
			Utils::execPython("add_sprite_to_showlist([" + argsStr + "], None)");
		}
	}else

	if (command == "hide") {
		if (params.startsWith("screen ")) {
			String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::addToHideSimply(screenName);
		}else {
			Utils::execPython("remove_sprite_from_showlist('" + params + "')");
		}
	}else

	if (command == "scene") {
		Utils::execPython("set_scene('" + params + "')");
	}else

	if (command == "with") {
		Utils::sleep(100);
	}else

	if (command == "window") {
		Utils::execPython("window_" + params + "()");
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
		Utils::execPython("pause_end = time.time() + (" + params + ")");
	}else

	if (command == "jump") {
		String label = params;
		if (label.startsWith("expression ")) {
			label = Utils::execPython(label.substr(String("expression ").size()), true);
		}

		jump(label);
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


	if (command == "menu") {
		String variants = children[0]->params;
		if (children.size() == 1) {
			variants += ", ";
		}else {
			for (size_t i = 1; i < children.size(); ++i) {
				variants += ", " + children[i]->params;
			}
		}
		Utils::execPython("choose_menu_variants = (" + variants + ")");
		Utils::execPython("renpy.call_screen('choose_menu', 'choose_menu_result')");

		int toSleep = Game::getFrameTime();
		while (GV::inGame && Utils::execPython("read", true) != "True") {
			Utils::sleep(toSleep);
		}
		if (!GV::inGame) return;

		String resStr = Utils::execPython("choose_menu_result", true);
		int res = resStr.toInt();

		if (res < 0 || res >= int(children.size())) {
			Utils::outMsg("Node::execute",
						  String() +
						  "Номер выбранного пункта меню находится вне допустимых пределов\n"
						  "choose_menu_result = " + res + ", min = 0, max = " + int(children.size())
						  );
			res = 0;
		}
		Node *menuItem = children[res];
		for (size_t i = 0; i < menuItem->children.size(); ++i) {
			Node *node = menuItem->children[i];
			node->execute();
		}
	}else

	if (command == "if") {
		String execRes = Utils::execPython(params, true);
		condIsTrue = execRes == "True";

		if (condIsTrue) {
			for (size_t i = 0; i < children.size(); ++i) {
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
			for (size_t i = 0; i < children.size(); ++i) {
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

		for (size_t i = 0; i < children.size(); ++i) {
			Node *node = children[i];
			node->execute();
		}
	}else

	if (command == "while") {
		condIsTrue = true;
		String cond = "bool(" + params + ")";
		while (GV::inGame && Utils::execPython(cond, true) == "True") {
			if (jumped || !GV::inGame) return;

			try {
				for (size_t i = 0; i < children.size(); ++i) {
					Node* node = children[i];
					node->execute();
				}
			}catch (ContinueException) {
			}catch (BreakException) {
				condIsTrue = false;
				break;
			}
		}
	}else

	if (command == "for") {
		condIsTrue = true;

		String in = " in ";
		size_t inPos = params.find(in);
		String beforeIn = params.substr(0, params.find_last_not_of(' ', inPos) + 1);
		String afterIn = params.substr(params.find_first_not_of(' ', inPos + in.size()));

		if (!beforeIn || !afterIn) {
			Utils::outMsg("Node::execute", "Неправильный синтаксис команды for:\n<" + params + ">");
			return;
		}

		static int numCicle = 0;
		String iterName = "iter_" + String(numCicle++);

		String init = iterName + " = iter(" + afterIn + ")";
		String onStep = beforeIn + " = " + iterName + ".next()";

		Utils::execPython(init);
		while (true) {
			if (jumped || !GV::inGame) return;

			try {
				Utils::execPython(onStep);

				for (size_t i = 0; i < children.size(); ++i) {
					Node* node = children[i];
					node->execute();
				}
			}catch (ContinueException) {
			}catch (BreakException) {
				condIsTrue = false;
				break;
			}catch (StopException) {
				break;
			}
		}
	}else

	if (command == "continue") {
		throw ContinueException();
	}else

	if (command == "break") {
		throw BreakException();
	}else

	{

		if (command != "pass") {
			Utils::outMsg("Node::execute: Неизвестный тип команды <" + command + ">");
		}
	}

	while (GV::inGame && !initing && Utils::execPython("pause_end > time.time()", true) == "True") {
		Utils::sleep(1);
	}

	int toSleep = Game::getFrameTime();
	while (GV::inGame && !initing && Utils::execPython("(not read) or (character_moving)", true) == "True") {
		Utils::sleep(toSleep);
	}
}

void Node::jump(const String &label) {
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
			String styleName = getPropCode("style");
			if (!styleName) {
				styleName = command;
			}
			res = Style::getProp(styleName, name);//command is a type of this screenChild
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
