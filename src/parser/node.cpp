#include "node.h"

#include <set>
#include <filesystem>

#include "gv.h"
#include "config.h"
#include "logger.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/image_manipulator.h"
#include "media/music.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/game.h"
#include "utils/mouse.h"
#include "utils/utils.h"

String Node::loadPath;

static const size_t NODES_IN_PART = 10000;
size_t Node::countNodes = 0;
std::vector<Node*> Node::nodes;

Node::Node(const String &fileName, size_t numLine, size_t id):
	fileName(fileName),
	numLine(numLine),
	id(id)
{}
Node::~Node() {}

Node* Node::getNewNode(const String &fileName, size_t numLine) {
	if ((countNodes % NODES_IN_PART) == 0) {
		nodes.push_back((Node*)::malloc(NODES_IN_PART * sizeof(Node)));
	}

	Node *node = nodes.back() + (countNodes % NODES_IN_PART);
	new(node) Node(fileName, numLine, countNodes);
	++countNodes;

	return node;
}
void Node::destroyAll() {
	for (size_t i = 0; i < nodes.size(); ++i) {
		const size_t size = i == nodes.size() - 1 ? (countNodes % NODES_IN_PART) : NODES_IN_PART;

		Node *array = nodes[i];
		for (size_t j = 0; j < size; ++j) {
			array[j].~Node();//destructor
		}
		::free(array);
	}
	countNodes = 0;
	nodes.clear();
}


bool Node::loading = false;
size_t Node::stackDepth = 0;
std::vector<std::pair<String, String>> Node::stack;

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
		if (!init && Node::stack.size()) {
			Node::stack.pop_back();
			--Node::stackDepth;
		}
	}
};


String Node::jumpNextLabel;
bool Node::jumpNextIsCall;


void Node::execute() {
	static bool initing = false;
	static bool inWithBlock = false;

	if (!GV::inGame) return;


	size_t curStackDepth = Node::stackDepth;
	if (!initing && curStackDepth < Node::stack.size()) {
		const std::pair<String, String> &cur = Node::stack[curStackDepth];

		const bool wasJump = cur.second == "jump" || cur.second == "call";
		const bool nowPython = command == "$" || command == "python";

		if (!(wasJump && nowPython)) {
			if (cur.second != command) {
				GV::inGame = false;
				Utils::outMsg(cur.second, command + " " + params);
				Utils::outMsg("Node::execute",
							  "Для загрузки сохранения необходима версия мода, в которой это сохранение было сделано");
				Node::stack.clear();
				Node::stackDepth = 0;
				return;
			}
		}
	}

	const size_t NO = size_t(-1);
	size_t nextNum = NO;
	String nextCommand;
	size_t nextStackDepth = curStackDepth + 1;
	if (!initing && nextStackDepth < Node::stack.size()) {
		const std::pair<String, String> &next = Node::stack[nextStackDepth];
		nextNum = next.first.toDouble();
		nextCommand = next.second;

		if (nextCommand != "label" && nextNum >= children.size()) {
			GV::inGame = false;
			Utils::outMsg("next", next.first + " " + next.second);
			Utils::outMsg("Node::execute",
						  "Для загрузки сохранения необходима версия мода, в которой это сохранение было сделано");
			Node::stack.clear();
			Node::stackDepth = 0;
			return;
		}
	}


	StackRecorder sr(initing, command == "label" ? params : String(childNum), command);




	if (command == "main") {
		Mouse::setCanHide(true);

		std::vector<Node*> initBlocks;
		for (Node *node : children) {
			if (node->command == "screen") {
				Screen::declare(node);
			}
			if (node->command == "init" || node->command == "init python") {
				initBlocks.push_back(node);
			}
		}
		std::sort(initBlocks.begin(), initBlocks.end(), [](Node* a, Node* b) { return a->priority < b->priority; });

		try {
			int initingStartTime = Utils::getTimer();
			initing = true;
			size_t i = 0;
			for (; i < initBlocks.size(); ++i) {
				initBlocks[i]->execute();
				if (Game::modStarting) break;
			}
			initing = false;
			Logger::logEvent("Mod Initing (" + String(i) + "/" + String(initBlocks.size()) + " blocks)", Utils::getTimer() - initingStartTime, true);

			if (Game::modStarting) return;

			std::vector<String> startScreensVec;
			if (loadPath) {
				PyUtils::exec("CPP_EMBED: node.cpp", __LINE__,
							  "load_global_vars('" + loadPath + "py_globals')");

				startScreensVec = Game::loadInfo(loadPath);
			}else if (0){
				std::lock_guard g(PyUtils::pyExecMutex);

				PyObject *startScreens = PyDict_GetItemString(PyUtils::global, "start_screens");
				if (startScreens) {
					if (PyList_CheckExact(startScreens)) {
						size_t len = Py_SIZE(startScreens);
						for (size_t i = 0; i < len; ++i) {
							PyObject *elem = PyList_GET_ITEM(startScreens, i);
							if (PyString_CheckExact(elem)) {
								String name = PyString_AS_STRING(elem);
								startScreensVec.push_back(name);
							}else {
								Utils::outMsg("Node::execute", "type(start_screens[" + String(i) + "]) is not str");
							}
						}
					}else {
						Utils::outMsg("Node::execute", "type(start_screens) is not list");
					}
				}else {
					Utils::outMsg("Node::execute", "Список start_screens не существует");
				}
			}

			for (const String &screenName : startScreensVec) {
				if (screenName) {
					Screen::addToShow(screenName);
				}
			}


			if (Game::hasLabel("start")) {
				try {
					jump("start", false);
				}catch (ExitException) {}
			}else {
				while (!Game::modStarting && !GV::exit) {
					Utils::sleep(Game::getFrameTime());
				}
			}

		}catch (ContinueException&) {
			Utils::outMsg("Node::execute", "continue вне цикла");
		}catch (BreakException&) {
			Utils::outMsg("Node::execute", "break вне цикла");
		}catch (StopException&) {
			Utils::outMsg("Node::execute", "Неожидаемое исключение StopException (конец итератора)");
		}

		if (!Game::modStarting && !GV::exit) {
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
			const std::vector<String> args = Algo::getArgs(params);
			if (args.empty()) {
				Utils::outMsg("Node::execute",
							  "Строка <show " + params + "> некорректна\n" +
							  getPlace());
				return;
			}

			String argsStr;
			for (size_t i = 0; i < args.size(); ++i) {
				argsStr += "'''" + args[i] + "''', ";
			}

			{
				std::lock_guard g(PyUtils::pyExecMutex);
				PyDict_SetItemString(PyUtils::global, "last_show_at", getPyList());
			}
			PyUtils::exec(getFileName(), getNumLine(), "show_sprite([" + argsStr + "], last_show_at)");
		}
	}else

	if (command == "hide") {
		if (params.startsWith("screen ")) {
			const String screenName = params.substr(params.find("screen ") + String("screen ").size());
			Screen::addToHide(screenName);
		}else {
			const std::vector<String> args = Algo::getArgs(params);
			if (args.empty()) {
				Utils::outMsg("Node::execute",
							  "Строка <hide " + params + "> некорректна\n" +
							  getPlace());
				return;
			}

			String argsStr;
			for (size_t i = 0; i < args.size(); ++i) {
				argsStr += "'''" + args[i] + "''', ";
			}

			PyUtils::exec(getFileName(), getNumLine(), "hide_sprite([" + argsStr + "])");
		}
	}else

	if (command == "scene") {
		const std::vector<String> args = Algo::getArgs(params);

		String argsStr;
		if (args.size()) {
			for (size_t i = 0; i < args.size(); ++i) {
				argsStr += "'''" + args[i] + "''', ";
			}
		}

		{
			std::lock_guard g(PyUtils::pyExecMutex);
			PyDict_SetItemString(PyUtils::global, "last_show_at", getPyList());
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

		if (params[0] != '"' && params[0] != '\'') {//"имя" указано?
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

		std::lock_guard g(PyUtils::pyExecMutex);

		PyObject *renpy = PyDict_GetItemString(PyUtils::global, "renpy");

		PyObject *pyNick = PyString_FromString(nick.c_str());
		PyObject *pyText = PyString_FromString(text.c_str());

		if (!PyObject_CallMethod(renpy, const_cast<char*>("say"),
								 const_cast<char*>("(OO)"), pyNick, pyText))
		{
			PyUtils::errorProcessing("renpy.say('" + nick + "', '" + text + "')");
		}

		Py_DECREF(pyNick);
		Py_DECREF(pyText);
	}else

	if (command == "$" || command == "python" || command == "init python") {
		if (nextNum == NO) {
			const size_t numLine = getNumLine() + (command != "$");

			PyUtils::exec(getFileName(), numLine, params);
			Utils::sleepMicroSeconds(50);
		}else {
			const std::pair<String, String> &cur = Node::stack[curStackDepth];
			const String &action = cur.second;

			const std::pair<String, String> &next = Node::stack[nextStackDepth];
			const String &label = next.first;

			jump(label, action == "call");
		}
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
			{
				std::lock_guard<std::mutex> g(GV::updateMutex);
				Screen::updateLists();
			}
			bool screenThereIs = Screen::getMain("choose_menu");

			if (!screenThereIs) {
				String variants;
				for (size_t i = 0; i < children.size(); ++i) {
					variants += children[i]->params + ", ";
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
		if (children.size()) {
			for (Node *child : children) {
				child->execute();
			}
		}else {
			const String &effectName = params;
			const String code = "screen.set_effect(" + effectName + ")";
			PyUtils::exec(getFileName(), getNumLine(), code);
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
		bool inCycle = nextNum != NO;

		condIsTrue = true;
		const String cond = "bool(" + params + ")";

		while (GV::inGame && (inCycle || PyUtils::exec(getFileName(), getNumLine(), cond, true) == "True")) {
			if (!GV::inGame) return;

			try {
				size_t i = inCycle ? nextNum : 0;
				for (; i < children.size(); ++i) {
					Node* node = children[i];
					node->execute();
				}
			}catch (ContinueException&) {
			}catch (BreakException&) {
				condIsTrue = false;
				break;
			}
			inCycle = false;
		}
	}else

	if (command == "for") {
		condIsTrue = true;
		bool inCycle = nextNum != NO;

		static const String in = " in ";
		const size_t inPos = params.find(in);
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

		const String init = iterName + " = iter(" + afterIn + ")";
		PyUtils::exec(getFileName(), getNumLine(), init);


		const String onStep = propName + " = " + iterName + ".next()";
		while (true) {
			if (!GV::inGame) break;

			try {
				if (!inCycle) {
					PyUtils::exec(getFileName(), getNumLine(), onStep);
				}

				size_t i = inCycle ? nextNum : 0;
				for (; i < children.size(); ++i) {
					Node* node = children[i];
					node->execute();
				}
			}catch (ContinueException&) {
			}catch (BreakException&) {
				condIsTrue = false;
				break;
			}catch (StopException&) {
				break;
			}
			inCycle = false;
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
		static const String code = "can_exec_next_command()";
		while (GV::inGame && PyUtils::exec("CPP_EMBED: node.cpp", __LINE__, code, true) == "False") {
			Utils::sleep(Game::getFrameTime());
		}
	}

	if (jumpNextLabel) {
		const String label = jumpNextLabel;
		jumpNextLabel = "";

		//for save/load
		std::pair<String, String> &last = Node::stack.back();
		last.second = jumpNextIsCall ? "call" : "jump";

		jump(label, jumpNextIsCall);
	}
}


void Node::jump(const String &label, bool isCall) {
	for (Node *node : GV::mainExecNode->children) {
		if (node->command == "label" && node->params == label) {
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

void Node::jumpNext(const std::string &label, bool isCall) {
	jumpNextLabel = label;
	jumpNextIsCall = isCall;
}


PyObject* Node::getPyList() const {
	PyObject *res = PyList_New(0);

	static const std::set<String> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = highLevelCommands.count(command);

	if (!isHighLevelCommands) {
		if (command) {
			PyList_Append(res, PyString_FromString(command.c_str()));
		}
		if (params) {
			PyList_Append(res, PyString_FromString(params.c_str()));
		}
	}

	for (const Node* child : children) {
		PyObject *childPyList = child->getPyList();

		static const std::set<String> blockCommandsInImage = {"contains", "block", "parallel"};
		bool childIsBlock = blockCommandsInImage.count(child->command);

		if (!childIsBlock) {
			std::vector<String> toJoin;
			size_t len = Py_SIZE(childPyList);
			for (size_t i = 0; i < len; ++i) {
				PyObject *elem = PyList_GET_ITEM(childPyList, i);
				const char *chars = PyString_AS_STRING(elem);
				toJoin.push_back(String(chars));
			}

			String joinedStr = String::join(toJoin, ' ');
			PyObject *joined = PyString_FromString(joinedStr.c_str());

			PyList_Append(res, joined);
		}else {
			PyList_Append(res, childPyList);
		}
	}

	return res;
}
std::vector<String> Node::getImageChildren() const {
	std::vector<String> res;

	static const std::vector<String> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = Algo::in(command, highLevelCommands);

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
		bool childIsBlock = Algo::in(child->command, blockCommandsInImage);

		if (!childIsBlock) {
			if (child->command || child->params) {
				const String str = Algo::clear(child->command + " " + child->params);
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
				if (t->command == "label" && t->params == label) {
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
			static const std::set<String> words = {"at", "with", "behind", "as"};

			--count;

			std::vector<String> args = Algo::getArgs(childParams);
			while (args.size() > 2 && words.count(args[args.size() - 2])) {
				args.erase(args.end() - 2, args.end());
			}
			if (args.empty()) continue;

			const String imageName = String::join(args, ' ');
			const String code = Utils::getImageCode(imageName);
			const String imageCode = PyUtils::exec(parent->getFileName(), parent->getNumLine(), code, true);
			if (imageCode) {
				ImageManipulator::loadImage(imageCode);
			}

			std::vector<String> declAt = Utils::getVectorImageDeclAt(imageName);
			preloadImageAt(declAt);
		}
	}

	return count;
}
void Node::preloadImageAt(const std::vector<String> &children) {
	auto fileExists = [](const String &path) -> bool {
		if (std::filesystem::exists(path.c_str())) {
			return !std::filesystem::is_directory(path.c_str());
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
						ImageManipulator::loadImage(imagePath);
					}
				}

				const std::vector<String> declAt = Utils::getVectorImageDeclAt(imageName);
				preloadImageAt(declAt);
			}else {
				if (fileExists(image)) {
					ImageManipulator::loadImage(image);
				}
			}
		}
	}
}

Node* Node::getProp(const String &name) {
	for (Node *child : children) {
		if (child->command == name) {
			return child;
		}
	}
	return nullptr;
}

String Node::getPlace() const {
	return "Место объявления объекта <" + command + ">:\n"
			"  Файл <" + fileName + ">\n" +
			"  Номер строки: " + String(numLine);
}

