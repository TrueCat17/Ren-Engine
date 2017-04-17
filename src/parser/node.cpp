#include "node.h"

#include <iostream>
#include <algorithm>

#include "gv.h"
#include "config.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/image.h"
#include "media/music.h"
#include "media/py_utils.h"

#include "utils/game.h"
#include "utils/utils.h"

std::vector<Node*> Node::nodes;

void Node::execute() {
	static bool initing = false;
	static bool inWithBlock = false;

	if (!GV::inGame) return;

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
			initing = true;
			for (Node *i : initBlocks) {
				i->execute();
			}
			initing = false;

			String startScreensStr = PyUtils::exec("CPP_EMBED: node.cpp", 0, "start_screens", true);
			std::vector<String> startScreensVec = startScreensStr.split(' ');
			for (String screenName : startScreensVec) {
				if (screenName) {
					Screen::addToShowSimply(screenName);
				}
			}

			mainLabel = PyUtils::exec("CPP_EMBED: node.cpp", 0, "mods_last_key", true);
			try {
				jump(mainLabel, false);
			}catch (ExitException) {}

		}catch (ContinueException) {
			Utils::outMsg("Node::execute", "continue вне цикла");
		}catch (BreakException) {
			Utils::outMsg("Node::execute", "break вне цикла");
		}catch (StopException) {
			Utils::outMsg("Node::execute", "Неожидаемое исключение StopException (конец итератора)");
		}

		if (!Game::modeStarting && !GV::exit) {
			Game::startMod("main_menu");
		}
	}else

	if (command == "init" || command == "label") {
		for (size_t i = 0; GV::inGame && i < children.size(); ++i) {
			Node *node = children[i];
			node->execute();

			if ((node->command == "show" && !node->params.startsWith("screen ")) || node->command == "scene") {
				preloadImages(this, i, Config::get("count_preload_commands").toInt());
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
				Utils::outMsg("Node::execute",
							  "Строка <show " + params + "> некорректна\n" +
							  getPlace());
				return;
			}

			String argsStr = "'" + args[0] + "'";
			for (size_t i = 1; i < args.size(); ++i) {
				argsStr += ", '" + args[i] + "'";
			}

			PyUtils::pyExecGuard.lock();
			GV::pyUtils->pythonGlobal["last_show_at"] = getPyList();
			PyUtils::pyExecGuard.unlock();

			PyUtils::exec(getFileName(), getNumLine(), "add_sprite_to_showlist([" + argsStr + "], last_show_at)");
		}
	}else

	if (command == "hide") {
		if (params.startsWith("screen ")) {
			String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::addToHideSimply(screenName);
		}else {
			const std::vector<String> args = Utils::getArgs(params);
			if (args.empty()) {
				Utils::outMsg("Node::execute",
							  "Строка <hide " + params + "> некорректна\n" +
							  getPlace());
				return;
			}

			String argsStr = "'" + args[0] + "'";
			for (size_t i = 1; i < args.size(); ++i) {
				argsStr += ", '" + args[i] + "'";
			}

			PyUtils::exec(getFileName(), getNumLine(), "add_sprite_to_hidelist([" + argsStr + "])");
		}
	}else

	if (command == "scene") {
		const std::vector<String> args = Utils::getArgs(params);

		String argsStr;
		if (args.size()) {
			argsStr = "'" + args[0] + "'";
			for (size_t i = 1; i < args.size(); ++i) {
				argsStr += ", '" + args[i] + "'";
			}
		}

		PyUtils::pyExecGuard.lock();
		GV::pyUtils->pythonGlobal["last_show_at"] = getPyList();
		PyUtils::pyExecGuard.unlock();

		PyUtils::exec(getFileName(), getNumLine(), "set_scene([" + argsStr + "], last_show_at)");
	}else

	if (command == "window") {
		PyUtils::exec(getFileName(), getNumLine(), "window_" + params + "()");
	}else

	if (command == "text") {
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
		PyUtils::exec(getFileName(), getNumLine(), code);
	}else

	if (command == "$") {
		PyUtils::exec(getFileName(), getNumLine(), params);
	}else

	if (command == "python" || command == "init python") {
		PyUtils::exec(getFileName(), getNumLine() + 1, params);
	}else

	if (command == "pause") {
		PyUtils::exec(getFileName(), getNumLine(), "pause_end = time.time() + (" + params + ")");
	}else

	if (command == "jump") {
		String label = params;
		if (label.startsWith("expression ")) {
			label = PyUtils::exec(getFileName(), getNumLine(), label.substr(String("expression ").size()), true);
		}

		jump(label, false);
	}else

	if (command == "call") {
		String label = params;
		if (label.startsWith("expression ")) {
			label = PyUtils::exec(getFileName(), getNumLine(), label.substr(String("expression ").size()), true);
		}

		jump(label, true);
	}else

	if (command == "play") {
		Music::play(params);
	}else

	if (command == "stop") {
		Music::stop(params);
	}else

	if (command == "image") {
		if (!Utils::registerImage(params, this)) {
			Utils::outMsg("Node::execute",
						  "Некорректное имя изображения\n" +
						  getPlace());
		}
	}else

	if (command == "nvl") {
		String code = "nvl_" + params + "()";
		PyUtils::exec(getFileName(), getNumLine(), code);
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
		PyUtils::exec(getFileName(), getNumLine(), "choose_menu_variants = (" + variants + ")");
		PyUtils::exec(getFileName(), getNumLine(), "renpy.call_screen('choose_menu', 'choose_menu_result')");

		while (GV::inGame && PyUtils::exec(getFileName(), getNumLine(), "menu_item_choosed", true) != "True") {
			Utils::sleep(Game::getFrameTime());
		}
		if (!GV::inGame) return;

		String resStr = PyUtils::exec(getFileName(), getNumLine(), "choose_menu_result", true);
		int res = resStr.toInt();

		if (res < 0 || res >= int(children.size())) {
			Utils::outMsg("Node::execute", String() +
						  "Номер выбранного пункта меню находится вне допустимых пределов\n"
						  "choose_menu_result = " + res + ", min = 0, max = " + children.size() + "\n" +
						  getPlace());
			res = 0;
		}
		Node *menuItem = children[res];
		for (size_t i = 0; i < menuItem->children.size(); ++i) {
			Node *node = menuItem->children[i];
			node->execute();
		}
	}else

	if (command == "with") {
		inWithBlock = true;
		for (Node *child : children) {
			child->execute();
		}
		inWithBlock = false;
	}else

	if (command == "if") {
		String execRes = PyUtils::exec(getFileName(), getNumLine(), "bool(" + params + ")", true);
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

		String execRes = PyUtils::exec(getFileName(), getNumLine(), "bool(" + params + ")", true);
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
		while (GV::inGame && PyUtils::exec(getFileName(), getNumLine(), cond, true) == "True") {
			if (!GV::inGame) return;

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

		static const String in = " in ";
		size_t inPos = params.find(in);
		String beforeIn = params.substr(0, params.find_last_not_of(' ', inPos) + 1);
		String afterIn = params.substr(params.find_first_not_of(' ', inPos + in.size()));

		if (!beforeIn || !afterIn) {
			Utils::outMsg("Node::execute",
						  "Неправильный синтаксис команды for:\n<" + params + ">\n" +
						  getPlace());
			return;
		}

		static int numCicle = 0;
		String iterName = "iter_" + String(numCicle++);

		String init = iterName + " = iter(" + afterIn + ")";
		String onStep = beforeIn + " = " + iterName + ".next()";

		PyUtils::exec(getFileName(), getNumLine(), init);
		while (true) {
			if (!GV::inGame) return;

			try {
				PyUtils::exec(getFileName(), getNumLine(), onStep);

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

	if (command == "return") {
		throw ReturnException();
	}else

	{

		if (command != "pass") {
			Utils::outMsg("Node::execute",
						  "Неизвестный тип команды <" + command + ">\n" +
						  getPlace());
		}
	}

	if (!initing && !inWithBlock) {
		static const String code = "pause_end > time.time() or not can_exec_next_command()";
		while (GV::inGame && PyUtils::exec("CPP_EMBED: node.cpp", 0, code, true) == "True") {
			Utils::sleep(Game::getFrameTime());
		}
	}
}
py::list Node::getPyList() const {
	py::list res;

	static const std::vector<String> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = Utils::in(command, highLevelCommands);

	if (!isHighLevelCommands) {
		if (command) {
			res.append(py::str(std::string(command)));
		}
		if (params) {
			res.append(py::str(std::string(params)));
		}
	}

	for (const Node* child : children) {
		py::list childPyList = child->getPyList();

		static const std::vector<String> blockCommandsInImage = {"contains", "parallel"};
		bool childIsBlock = Utils::in(child->command, blockCommandsInImage);

		static const py::str space = " ";

		if (!childIsBlock) {
			res.append(space.join(childPyList));
		}else {
			res.append(childPyList);
		}
	}

	return res;
}

void Node::jump(const String &label, bool isCall) {
	for (size_t i = 0; i < GV::mainExecNode->children.size(); ++i) {
		Node *node = GV::mainExecNode->children[i];
		if (node->command == "label" && node->name == label) {
			try {
				node->execute();
			}catch (ReturnException) {}

			if (!isCall) {
				throw ExitException();
			}
			return;
		}
	}
	Utils::outMsg("Node::jump(call)",
				  "Метка <" + label + "> не найдена\n");
}
void Node::preloadImages(Node *node, int start, int count) {
	return;

	for (size_t i = start; i < node->children.size() && count > 0; ++i) {
		Node *child = node->children[i];
		String childCommand = child->command;
		String childParams = child->params;

		if ((childCommand == "show" && !childParams.startsWith("screen ")) || childCommand == "scene") {
			static std::vector<String> words = {"at", "with", "behind", "as"};

			--count;

			std::vector<String> args = Utils::getArgs(childParams);
			while (args.size() > 2 && Utils::in(args[args.size() - 2], words)) {
				args.erase(args.end() - 2, args.end());
			}
			if (args.empty()) continue;

			String imageName = args[0];
			for (size_t j = 1; j < args.size(); ++j) {
				imageName += ' ' + args[j];
			}
			String imageCode = PyUtils::exec(node->getFileName(), node->getNumLine(), Utils::getImageCode(imageName), true);
			if (!imageCode) return;

			auto loadImage = [=]() {
				Image::getImage(imageCode);
			};
			std::thread(loadImage).detach();
		}else

		if (childCommand == "jump" || childCommand == "call") {
			String label = childParams;
			for (size_t j = 0; j < GV::mainExecNode->children.size(); ++j) {
				Node *t = GV::mainExecNode->children[j];
				if (t->command == "label" && t->name == label) {
					i = -1;
					node = t;
					break;
				}
			}
		}else

		if (childCommand == "if" || childCommand == "elif" || childCommand == "else") {
			preloadImages(child, 0, 1);
		}
	}
}

void Node::initProp(const String &name, const String &value, size_t numLine) {
	props[name] = { value, numLine };
}

String Node::getProp(const String &name, const String &commonName, const String &indexStr) const {
	static std::vector<String> exceptions = String("if elif for while else hotspot").split(' ');
	if (Utils::in(command, exceptions)) {
		return "None";
	}

	String res;
	auto i = props.find(name);
	if (i != props.end()) {
		String toExec = i->second.pyExpr;
		res = PyUtils::exec(getFileName(), i->second.numLine, toExec, true);
		return res;
	}

	if (commonName) {
		i = props.find(commonName);
		if (i != props.end()) {
			String toExec = i->second.pyExpr;
			res = PyUtils::exec(getFileName(), i->second.numLine, toExec + indexStr, true);
			return res;
		}
	}

	String styleName = getPropCode("style");
	if (!styleName) {
		styleName = command;
	}
	res = Style::getProp(styleName, name);
	return res;
}
String Node::getPropCode(const String &name) const {
	auto i = props.find(name);
	if (i == props.end()) return "";

	const String& res = i->second.pyExpr;
	return res;
}
String Node::getPlace() const {
	return "Место объявления объекта <" + command + ">:\n"
			"  Файл <" + fileName + ">\n" +
			"  Номер строки: " + String(numLine);
}


void Node::destroyAll() {
	for (size_t i = 0; i < nodes.size(); ++i) {
		Node *node = nodes[i];
		delete node;
	}
	nodes.clear();
}
