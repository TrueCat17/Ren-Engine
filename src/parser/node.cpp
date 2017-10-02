#include "node.h"

#include <thread>
#include <algorithm>

#include <boost/filesystem.hpp>

#include "gv.h"
#include "config.h"
#include "logger.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/image.h"
#include "media/music.h"
#include "media/py_utils.h"

#include "utils/game.h"


bool Node::loading = false;
size_t Node::stackDepth = 0;
std::vector<std::pair<String, String>> Node::stack;

const String Node::mainLabel = "start";

class StackRecorder {
private:
	bool init;

public:
	StackRecorder(bool init, String i, String s):
		init(init)
	{
		if (!init) {
			if (!Node::loading) {
				Node::stack.push_back(std::make_pair(i, s));
			}

			++Node::stackDepth;
			if (Node::stackDepth == Node::stack.size()) {
				Node::loading = false;//now loaded
			}
		}
	}
	~StackRecorder() {
		if (!init) {
			Node::stack.pop_back();
			--Node::stackDepth;
		}
	}
};


std::vector<Node*> Node::nodes;

void Node::execute() {
	static bool initing = false;
	static bool inWithBlock = false;

	if (!GV::inGame) return;


	size_t curStackDepth = Node::stackDepth;
	if (!initing && curStackDepth < Node::stack.size()) {
		const std::pair<String, String> &cur = Node::stack.at(curStackDepth);
		if (cur.second != command) {
			GV::inGame = false;
			Utils::outMsg(cur.second, command);
			Utils::outMsg("Node::execute",
						  "Для загрузки сохранения необходима версия мода, в которой это сохранение было сделано");
			return;
		}
	}

	const size_t NO = size_t(-1);
	size_t nextNum = NO;
	String nextCommand;
	size_t nextStackDepth = curStackDepth + 1;
	if (!initing && nextStackDepth < Node::stack.size()) {
		const std::pair<String, String> &next = Node::stack.at(nextStackDepth);
		nextNum = next.first.toDouble();
		nextCommand = next.second;

		if (nextCommand != "label" && nextNum >= children.size()) {
			GV::inGame = false;
			Utils::outMsg("next", next.first + " " + next.second);
			Utils::outMsg("Node::execute",
						  "Для загрузки сохранения необходима версия мода, в которой это сохранение было сделано");
			return;
		}
	}


	StackRecorder sr(initing, command == "label" ? name : String(childNum), command);




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
			int initingStartTime = Utils::getTimer();
			initing = true;
			for (Node *i : initBlocks) {
				i->execute();
			}
			initing = false;
			Logger::logEvent("Mod Initing (" + String(initBlocks.size()) + " blocks)", Utils::getTimer() - initingStartTime, true);


			std::vector<String> startScreensVec;
			if (loadPath) {
				PyUtils::exec("CPP_EMBED: node.cpp", __LINE__,
							  "load_global_vars('" + loadPath + "py_globals" + "')");

				startScreensVec = Game::loadInfo(loadPath);
			}else {
				String startScreens = PyUtils::exec("CPP_EMBED: node.cpp", __LINE__, "start_screens", true);
				startScreensVec = startScreens.split(' ');
			}

			for (const String &screenName : startScreensVec) {
				if (screenName) {
					Screen::addToShow(screenName);
				}
			}


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

	if (command == "init" || command == "label" || command == "menuItem") {
		bool isInit = command == "init";
		size_t i = (!isInit && nextNum != NO) ? nextNum : 0;

		for (; GV::inGame && i < children.size(); ++i) {
			Node *node = children[i];
			node->execute();

			if (!isInit) {
				if ((node->command == "show" && !node->params.startsWith("screen ")) ||
					node->command == "scene" ||
					node->command == "with"  ||
					!i)
				{
					preloadImages(this, i + 1, Config::get("count_preload_commands").toInt());
				}
			}
		}
	}else

	if (command == "show") {
		if (params.startsWith("screen ")) {
			const String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::addToShow(screenName);
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

			{
				std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
				GV::pyUtils->pythonGlobal["last_show_at"] = getPyList();
			}
			PyUtils::exec(getFileName(), getNumLine(), "show_sprite([" + argsStr + "], last_show_at)");
		}
	}else

	if (command == "hide") {
		if (params.startsWith("screen ")) {
			const String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::addToHide(screenName);
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

			PyUtils::exec(getFileName(), getNumLine(), "hide_sprite([" + argsStr + "])");
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

		{
			std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
			GV::pyUtils->pythonGlobal["last_show_at"] = getPyList();
		}

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
		String textCode = params.substr(startText, endText - startText);
		String text = PyUtils::exec(getFileName(), getNumLine(), textCode, true);

		if (!nick) {
			nick = "narrator";
		}

		std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
		try {
			py::dict global = py::extract<py::dict>(GV::pyUtils->pythonGlobal);
			if (global.has_key("renpy")) {
				py::object renpy = global["renpy"];
				py::object say = renpy.attr("say");
				say(nick.c_str(), text.c_str());
			}
		}catch (py::error_already_set) {
			PyUtils::errorProcessing("renpy.say(" + nick + ", " + text + ")");
		}
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

	if (command == "jump" || command == "call") {
		String label = params;
		if (label.startsWith("expression ")) {
			label = PyUtils::exec(getFileName(), getNumLine(), label.substr(String("expression ").size()), true);
		}

		jump(label, command == "call");
	}else

	if (command == "play") {
		Music::play(params, fileName, numLine);
	}else

	if (command == "stop") {
		Music::stop(params, fileName, numLine);
	}else

	if (command == "image") {
		if (!Utils::registerImage(params, this)) {
			Utils::outMsg("Node::execute",
						  "Некорректное имя изображения\n" +
						  getPlace());
		}
	}else

	if (command == "nvl") {
		const String code = "nvl_" + params + "()";
		PyUtils::exec(getFileName(), getNumLine(), code);
	}else


	if (command == "menu") {
		size_t choose = nextNum;

		if (choose == NO) {
			//loaded with screen choose_menu?
			Screen::updateLists();
			bool screenThereIs = Screen::getMain("choose_menu");

			if (!screenThereIs) {
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
			}

			while (GV::inGame && PyUtils::exec(getFileName(), getNumLine(), "call_screen_choosed", true) != "True") {
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
			choose = res;
		}

		Node *menuItem = children[choose];
		menuItem->execute();
	}else

	if (command == "with") {
		inWithBlock = true;
		for (Node *child : children) {
			child->execute();
		}
		inWithBlock = false;
	}else

	if (command == "if") {
		if (nextNum == NO) {
			nextNum = 0;

			const String execRes = PyUtils::exec(getFileName(), getNumLine(), "bool(" + params + ")", true);
			condIsTrue = execRes == "True";
		}else {
			condIsTrue = true;
		}

		if (condIsTrue) {
			for (size_t i = nextNum; i < children.size(); ++i) {
				Node *node = children[i];
				node->execute();
			}
		}
	}else

	if (command == "elif") {
		if (nextNum == NO) {
			nextNum = 0;

			const Node *t = prevNode;
			while (t->command != "if" && !t->condIsTrue) {
				t = t->prevNode;
			}
			if (t->condIsTrue) return;

			const String execRes = PyUtils::exec(getFileName(), getNumLine(), "bool(" + params + ")", true);
			condIsTrue = execRes == "True";
		}else {
			condIsTrue = true;
		}

		if (condIsTrue) {
			for (size_t i = nextNum; i < children.size(); ++i) {
				Node *node = children[i];
				node->execute();
			}
		}
	}else

	if (command == "else") {
		if (nextNum == NO) {
			nextNum = 0;

			const Node *t = prevNode;
			while (t->command != "if" && !t->condIsTrue) {
				t = t->prevNode;
			}
			if (t->condIsTrue) return;
		}

		for (size_t i = nextNum; i < children.size(); ++i) {
			Node *node = children[i];
			node->execute();
		}
	}else

	if (command == "while") {
		bool inCicle = nextNum != NO;

		condIsTrue = true;
		const String cond = "bool(" + params + ")";

		while (GV::inGame && (inCicle || PyUtils::exec(getFileName(), getNumLine(), cond, true) == "True")) {
			if (!GV::inGame) return;

			try {
				size_t i = inCicle ? nextNum : 0;
				for (; i < children.size(); ++i) {
					Node* node = children[i];
					node->execute();
				}
			}catch (ContinueException) {
			}catch (BreakException) {
				condIsTrue = false;
				break;
			}
			inCicle = false;
		}
	}else

	if (command == "for") {
		condIsTrue = true;
		bool inCicle = nextNum != NO;

		static const String in = " in ";
		size_t inPos = params.find(in);
		const String beforeIn = params.substr(0, params.find_last_not_of(' ', inPos) + 1);
		const String afterIn = params.substr(params.find_first_not_of(' ', inPos + in.size()));

		if (!beforeIn || !afterIn) {
			Utils::outMsg("Node::execute",
						  "Неправильный синтаксис команды for:\n<" + params + ">\n" +
						  getPlace());
			return;
		}

		const String &propName = beforeIn;
		const String iterName = "iter_" + String(GV::numFor++);

		String init = iterName + " = iter(" + afterIn + ")";
		PyUtils::exec(getFileName(), getNumLine(), init);


		String onStep = propName + " = " + iterName + ".next()";
		while (true) {
			if (!GV::inGame) break;

			try {
				if (!inCicle) {
					PyUtils::exec(getFileName(), getNumLine(), onStep);
				}

				size_t i = inCicle ? nextNum : 0;
				for (; i < children.size(); ++i) {
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
			inCicle = false;
		}

		PyUtils::exec(getFileName(), getNumLine(), "del " + iterName);
		--GV::numFor;
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
		while (GV::inGame && PyUtils::exec("CPP_EMBED: node.cpp", __LINE__, code, true) == "True") {
			Utils::sleep(Game::getFrameTime());
		}
	}
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

		static const std::vector<String> blockCommandsInImage = {"contains", "block", "parallel"};
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
std::vector<String> Node::getImageChildren() const {
	std::vector<String> res;

	static const std::vector<String> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = Utils::in(command, highLevelCommands);

	if (!isHighLevelCommands) {
		if (command) {
			res.push_back(command);
		}
		if (params) {
			res.push_back(params);
		}
	}

	for (const Node* child : children) {
		static const std::vector<String> blockCommandsInImage = {"contains", "block", "parallel"};
		bool childIsBlock = Utils::in(child->command, blockCommandsInImage);

		if (!childIsBlock) {
			if (child->command || child->params) {
				const String str = Utils::clear(child->command + " " + child->params);
				res.push_back(str);
			}
		}else {
			const std::vector<String> childChildren = child->getImageChildren();
			for (const String &str : childChildren) {
				res.push_back(str);
			}
		}
	}

	return res;
}



int Node::preloadImages(const Node *parent, int start, int count) {
	for (size_t i = start; i < parent->children.size() && count > 0; ++i) {
		const Node *child = parent->children[i];
		const String &childCommand = child->command;
		const String &childParams = child->params;

		if (childCommand == "with") {
			count = preloadImages(child, 0, count);
		}else

		if (childCommand == "jump" || childCommand == "call") {
			const String &label = childParams;

			for (size_t j = 0; j < GV::mainExecNode->children.size(); ++j) {
				Node *t = GV::mainExecNode->children[j];
				if (t->command == "label" && t->name == label) {
					if (childCommand == "jump") {
						return preloadImages(child, 0, count - 1);
					}else {
						--count;
						preloadImages(child, 0, 1);
					}
				}
			}
		}else

		if (childCommand == "if" || childCommand == "elif" || childCommand == "else") {
			preloadImages(child, 0, 1);
		}else



		if ((childCommand == "show" && !childParams.startsWith("screen ")) ||
			childCommand == "scene")
		{
			static const std::vector<String> words = {"at", "with", "behind", "as"};

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


			const String code = Utils::getImageCode(imageName);
			const String imageCode = PyUtils::exec(parent->getFileName(), parent->getNumLine(), code, true);
			if (imageCode) {
				Image::loadImage(imageCode);
			}

			std::vector<String> declAt = Utils::getVectorImageDeclAt(imageName);
			preloadImageAt(declAt);
		}
	}

	return count;
}
void Node::preloadImageAt(const std::vector<String> &children) {
	auto fileExists = [](const String &path) -> bool {
		if (boost::filesystem::exists(path.c_str())) {
			return !boost::filesystem::is_directory(path.c_str());
		}
		return false;
	};

	for (const String &str : children) {
		if (str.isSimpleString()) {
			const String image = str.substr(1, str.size() - 2);

			if (Utils::imageWasRegistered(image)) {
				const String &imageName = image;

				const std::pair<String, size_t> place = Utils::getImagePlace(imageName);

				const String code = Utils::getImageCode(imageName);
				const String imagePath = PyUtils::exec(place.first, place.second, code, true);
				if (imagePath) {
					if (fileExists(imagePath)) {
						Image::loadImage(imagePath);
					}
				}

				const std::vector<String> declAt = Utils::getVectorImageDeclAt(imageName);
				preloadImageAt(declAt);
			}else {
				if (fileExists(image)) {
					Image::loadImage(image);
				}
			}
		}
	}
}


void Node::initProp(const String &name, const String &value, size_t numLine) {
	props[name] = NodeProp::initPyExpr(value, numLine);
}

NodeProp Node::getPropCode(const String &name, const String &commonName, const String &indexStr) const {
	static std::vector<String> exceptions = String("if elif for while else").split(' ');
	if (Utils::in(command, exceptions)) {
		return NodeProp::initPyExpr("None", 0);
	}

	auto i = props.find(name);
	if (i != props.end()) {
		return NodeProp::initPyExpr(i->second.pyExpr, i->second.numLine);
	}

	if (commonName) {
		i = props.find(commonName);
		if (i != props.end()) {
			return NodeProp::initPyExpr(i->second.pyExpr + indexStr, i->second.numLine);
		}
	}

	i = props.find("style");
	String styleName = i == props.end() ? command : i->second.pyExpr;
	return NodeProp::initStyleProp(styleName, name);
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
