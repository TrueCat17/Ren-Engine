#include "scenario.h"

#include <algorithm>
#include <fstream>


#include "config.h"
#include "gv.h"
#include "logger.h"

#include "gui/screen/screen.h"

#include "media/music.h"
#include "media/py_utils.h"
#include "media/translation.h"

#include "parser/node.h"

#include "utils/algo.h"
#include "utils/game.h"
#include "utils/mouse.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"



static std::map<std::string, Node*> declaredLabels;

static void declareLabel(Node *labelNode) {
	declaredLabels[labelNode->params] = labelNode;
}
static Node* getLabel(const std::string &name, bool outError = true) {
	auto it = declaredLabels.find(name);
	if (it != declaredLabels.end()) return it->second;

	if (outError) {
		Utils::outMsg("Scenario::getLabel", "Label <" + name + "> not found");
	}
	return nullptr;
}
static void markLabel(const std::string &fileName, size_t numLine, const std::string &label) {
	PyUtils::exec(fileName, numLine,
	              "persistent._seen_labels[get_current_mod()]['''" + label + "'''] = True");
}


static std::vector<std::pair<Node*, size_t>> stack;

static void restoreStack(const std::string &loadPath) {
	stack.clear();
	if (loadPath.empty()) {
		if (Game::hasLabel("start")) {
			stack.push_back({getLabel("start"), 0});
		}

		Game::setFps(60);
		Mouse::setCanHide(true);
		Game::setCanAutoSave(true);
		return;
	}

	std::ifstream is(loadPath + "/stack");

	Node *prevNode = nullptr;
	size_t num = 0;

	while (!is.eof()) {
		std::string tmp;
		std::getline(is, tmp);
		if (tmp.empty()) return;

		std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 2) {
			Utils::outMsg("Scenario::restoreStack", "In string <" + tmp + "> expected 2 arguments");
			return;
		}

		std::string first = tmpVec[0];
		std::string second = tmpVec[1];

		Node *node;

		if (String::startsWith(first, "label:")) {
			std::string labelName = first.substr(strlen("label:"));
			node = getLabel(labelName);
			if (!node) return;

			num = size_t(String::toInt(second));

			stack.push_back({node, num});
			prevNode = node;
			continue;
		}


		if (!prevNode) {
			Utils::outMsg("Scenario::restoreStack", "Label not defined");
			return;
		}

		if (num - 1 >= prevNode->children.size()) {
			Utils::outMsg(
			    "Scenario::restoreStack",
			    "Node have only " + std::to_string(prevNode->children.size()) + " children, "
			      "need #" + std::to_string(num - 1) + "\n" +
			    prevNode->getPlace()
			);
			return;
		}

		node = prevNode->children[num - 1];
		if (node->command != first) {
			Utils::outMsg(
			    "Scenario::restoreStack",
			    "Expected command <" + first + ">, got <" + node->command + ">\n" +
			    node->getPlace()
			);
			return;
		}

		num = size_t(String::toInt(second));

		stack.push_back({node, num});
		prevNode = node;
	}
}

static bool needToSaveStack = false;
static std::vector<std::pair<std::string, std::string>> stackToSave;
std::vector<std::pair<std::string, std::string> > Scenario::getStackToSave() {
	needToSaveStack = true;
	while (needToSaveStack && GV::inGame) {
		Utils::sleep(Game::getFrameTime());
	}
	return stackToSave;
}
static void checkToSaveStack() {
	if (!needToSaveStack) return;

	stackToSave.clear();
	stackToSave.reserve(stack.size());

	for (auto &[node, num] : stack) {
		if (node->command == "label") {
			stackToSave.push_back({"label:" + node->params, std::to_string(num)});
		}else {
			stackToSave.push_back({node->command, std::to_string(num)});
		}
	}

	needToSaveStack = false;
}


static void restoreScreens(const std::string &loadPath) {
	std::vector<std::string> startScreensVec;
	if (!loadPath.empty()) {
		PyUtils::exec("CPP_EMBED: scenario.cpp", __LINE__,
		              "load_global_vars('" + loadPath + "/py_globals')");

		startScreensVec = Game::loadInfo(loadPath);
	}else {
		std::lock_guard g(PyUtils::pyExecMutex);

		PyObject *startScreens = PyDict_GetItemString(PyUtils::global, "start_screens");
		if (startScreens) {
			if (PyList_CheckExact(startScreens)) {
				size_t len = size_t(Py_SIZE(startScreens));
				for (size_t i = 0; i < len; ++i) {
					PyObject *elem = PyList_GET_ITEM(startScreens, i);
					if (PyString_CheckExact(elem)) {
						std::string name = PyString_AS_STRING(elem);
						startScreensVec.push_back(name);
					}else {
						Utils::outMsg("Scenario::execute",
						              "type(start_screens[" + std::to_string(i) + "]) is not str");
					}
				}
			}else {
				Utils::outMsg("Scenario::execute", "type(start_screens) is not list");
			}
		}else {
			Utils::outMsg("Scenario::execute", "start_screens is not defined");
		}
	}

	for (const std::string &screenName : startScreensVec) {
		if (!screenName.empty()) {
			Screen::addToShow(screenName);
		}
	}
}



static std::string jumpNextLabel;
static bool jumpNextIsCall;

void Scenario::jumpNext(const std::string &label, bool isCall) {
	jumpNextLabel = label;
	jumpNextIsCall = isCall;
}


void Scenario::execute(const std::string &loadPath) {
	long initingStartTime = Utils::getTimer();
	size_t initNum = 0;

	ScopeExit se([]() {
		declaredLabels.clear();
	});

	std::vector<Node*> initBlocks;
	for (Node *node : GV::mainExecNode->children) {
		if (node->command == "init" || node->command == "init python") {
			initBlocks.push_back(node);
		}else
		if (node->command == "label") {
			declareLabel(node);
		}else
		if (node->command == "screen") {
			Screen::declare(node);
		}
	}
	std::stable_sort(initBlocks.begin(), initBlocks.end(), [](Node* a, Node* b) {
		return a->priority < b->priority;
	});

	restoreStack(loadPath);
	for (auto it = initBlocks.rbegin(); it < initBlocks.rend(); ++it) {
		stack.push_back({*it, 0});
	}


	std::pair<Node*, size_t> elem = stack.back();
	Node *obj = elem.first;
	size_t num = elem.second;

	bool initing = true;
	bool inWithBlock = false;

	jumpNextLabel.clear();

	while (true) {
		if (!jumpNextLabel.empty()) {
			const std::string label = jumpNextLabel;
			jumpNextLabel.clear();

			Node *labelNode = getLabel(label);
			if (!labelNode) continue;

			if (!jumpNextIsCall) {
				stack.clear();
			}

			obj = labelNode;
			num = 0;
			stack.push_back({obj, num});
			continue;
		}

		if (!GV::inGame || !obj) break;


		if (!initing && !inWithBlock) {
			checkToSaveStack();

			static const std::string code = "can_exec_next_command()";
			while (GV::inGame && PyUtils::exec("CPP_EMBED: scenario.cpp", __LINE__, code, true) == "False") {
				Utils::sleep(Game::getFrameTime());
				checkToSaveStack();
			}
			if (!GV::inGame) return;
		}


		if (num == obj->children.size()) {
			if (obj->command == "init" || obj->command == "init python") {
				if (obj->command == "init python") {
					PyUtils::exec(obj->getFileName(), obj->getNumLine() + 1, obj->params);
				}

				++initNum;
				if (initNum == initBlocks.size()) {
					Translation::enable();
					PyUtils::exec("CPP_EMBED: scenario.cpp", __LINE__, "_choose_lang()");

					restoreScreens(loadPath);
					initing = false;
					Logger::logEvent("Mod Initing (" + std::to_string(initBlocks.size()) + " blocks)",
					                 Utils::getTimer() - initingStartTime, true);

					if (Game::hasLabel("start")) {
						Node *start = getLabel("start");
						markLabel(start->getFileName(), start->getNumLine(), "start");
					}
				}
			}else

			if (obj->command == "while") {
				std::string code = "bool(" + obj->params + ")";
				if (PyUtils::exec(obj->getFileName(), obj->getNumLine(), code, true) == "True") {
					num = stack.back().second = 0;
					continue;
				}
			}else

			if (obj->command == "with") {
				inWithBlock = false;
			}else

			if (obj->command == "menuItem") {
				stack.pop_back();
				if (stack.empty()) {
					Utils::outMsg("Scenario::execute", "menuItem is first in stack");
					break;
				}
			}

			stack.pop_back();
			if (stack.empty()) {
				obj = nullptr;
				continue;
			}

			Node *prevObj = obj;

			elem = stack.back();
			obj = elem.first;
			num = elem.second;

			if (String::endsWith(prevObj->command, "if")) {//<if>, <elif>
				while (num < obj->children.size() && String::startsWith(obj->children[num]->command, "el")) {//<elif>, <else>
					++num;
					++stack.back().second;
				}
			}

			continue;
		}


		if (obj->command == "menu") {
			const std::string resStr = PyUtils::exec(obj->getFileName(), obj->getNumLine(), "int(choose_menu_result)", true);
			int res = String::toInt(resStr);

			if (res < 0 || res >= int(obj->children.size())) {
				Utils::outMsg("Scenario::execute",
				              "choose_menu_result = " + resStr + ", min = 0, max = " + std::to_string(obj->children.size()) + "\n" +
				              obj->getPlace());
				res = 0;
			}

			obj = obj->children[size_t(res)];
			num = 0;
			stack.push_back({obj, num});
			continue;
		}



		Node *child = obj->children[num];
		++num;
		++stack.back().second;

		if (child->command == "menu") {
			//if loading
			{
				std::lock_guard g(GV::updateMutex);
				Screen::updateLists();
			}
			bool screenThereIs = Screen::getMain("choose_menu");
			if (!screenThereIs) {
				std::string variants;
				for (size_t i = 0; i < child->children.size(); ++i) {
					variants += child->children[i]->params + ", ";
				}

				PyUtils::exec(child->getFileName(), child->getNumLine(), "choose_menu_variants = (" + variants + ")");
				PyUtils::exec(child->getFileName(), child->getNumLine(), "renpy.call_screen('choose_menu', 'choose_menu_result')");
			}

			obj = child;
			num = 0;

			stack.push_back({obj, num});
			continue;
		}

		if (child->command == "image") {
			Utils::registerImage(child);
			continue;
		}

		if (child->command == "scene" || child->command == "show" || child->command == "hide") {
			std::string funcName;
			if (child->command == "scene") {
				funcName = "set_scene";
			}else {
				if (String::startsWith(child->params, "screen ")) {
					const std::string screenName = child->params.substr(strlen("screen "));
					if (child->command == "show") {
						Screen::addToShow(screenName);
					}else {
						Screen::addToHide(screenName);
					}
					continue;
				}else {
					funcName = child->command + "_sprite";
				}
			}

			const std::vector<std::string> args = Algo::getArgs(child->params);
			if (args.empty() && child->command != "scene") {
				Utils::outMsg("Scenario::execute",
				              "Invalid command <" + child->command + " " + child->params + ">\n" +
				              child->getPlace());
				continue;
			}

			std::string argsStr = "[";
			for (size_t i = 0; i < args.size(); ++i) {
				argsStr += "'''" + args[i] + "''', ";
			}
			argsStr += "]";

			if (child->command != "hide") {
				Node::preloadImages(obj, num, size_t(String::toInt(Config::get("count_preload_commands"))));

				argsStr += ", last_show_at";

				std::lock_guard g(PyUtils::pyExecMutex);
				PyDict_SetItemString(PyUtils::global, "last_show_at", child->getPyList());
			}

			PyUtils::exec(child->getFileName(), child->getNumLine(), funcName + "(" + argsStr + ")");
			continue;
		}



		if (!child->children.empty()) {
			size_t newNum = 0;

			if (child->command == "if" || child->command == "elif") {
				if (PyUtils::exec(child->getFileName(), child->getNumLine(),
				                  "bool(" +child->params + ")", true) == "False")
				{
					continue;
				}
			}else

			if (child->command == "while") {
				newNum = child->children.size();
			}else

			if (child->command == "with") {
				inWithBlock = true;
			}

			obj = child;
			num = newNum;

			stack.push_back({obj, num});
			continue;
		}



		if (child->command == "$" || child->command == "python") {
			const size_t numLine = child->getNumLine() + (child->command != "$");

			PyUtils::exec(child->getFileName(), numLine, child->params);
			Utils::sleepMicroSeconds(50);
			continue;
		}

		if (child->command == "pause") {
			PyUtils::exec(child->getFileName(), child->getNumLine(), "pause(" + child->params + ")");
			continue;
		}

		if (child->command == "jump" || child->command == "call") {
			std::string label = child->params;
			if (String::startsWith(label, "expression ")) {
				label = PyUtils::exec(child->getFileName(), child->getNumLine(),
				                      label.substr(strlen("expression ")), true);
			}

			Node *labelNode = getLabel(label);
			if (!labelNode) continue;

			if (child->command == "jump") {
				stack.clear();
			}

			markLabel(child->getFileName(), child->getNumLine(), label);

			obj = labelNode;
			num = 0;
			stack.push_back({obj, num});
			continue;
		}

		if (child->command == "window") {
			PyUtils::exec(child->getFileName(), child->getNumLine(), "window_" + child->params + "()");
			continue;
		}

		if (child->command == "nvl") {
			const std::string code = "nvl_" + child->params + "()";
			PyUtils::exec(child->getFileName(), child->getNumLine(), code);
			continue;
		}

		if (child->command == "text") {
			size_t startText = 0;
			std::string nick;

			if (child->params[0] != '"' && child->params[0] != '\'') {//Is character defined?
				size_t space = child->params.find(' ');
				startText = child->params.find_first_not_of(' ', space);

				nick = child->params.substr(0, space);
			}else {
				nick = "narrator";
			}
			std::string textCode = child->params.substr(startText);


			std::string tmp = (startText ? nick + " " : "") + textCode + "\r\n";
			std::string md5 = PyUtils::getMd5(tmp);

			size_t i = stack.size() - 1;
			while (stack[i].first->command != "label") {
				--i;
			}
			std::string curLabel = stack[i].first->params;

			std::string translateLabelName = Translation::getLang() + " " + curLabel + "_" + md5.substr(0, 8);
			Node *translateLabel = getLabel(translateLabelName, false);
			if (translateLabel) {
				obj = translateLabel;
				num = 0;
				stack.push_back({obj, num});
			}else {
				PyUtils::exec(child->getFileName(), child->getNumLine(),
				              "renpy.say(" + nick + ", " + textCode + ")");
			}
			continue;
		}

		if (child->command == "play") {
			Music::play(child->params, child->getFileName(), child->getNumLine());
			continue;
		}

		if (child->command == "stop") {
			Music::stop(child->params, child->getFileName(), child->getNumLine());
			continue;
		}


		if (child->command == "continue" || child->command == "break") {
			while (obj->command != "while") {
				stack.pop_back();
				if (stack.empty()) {
					Utils::outMsg("Scenario::execute", child->command + " outside loop\n" + child->getPlace());
					break;
				}

				elem = stack.back();
				obj = elem.first;
			}

			if (child->command == "continue") {
				num = stack.back().second = obj->children.size();
			}else {
				stack.pop_back();
				if (stack.empty()) {
					Utils::outMsg("Scenario::execute", child->command + " outside loop\n" + child->getPlace());
					break;
				}

				elem = stack.back();
				obj = elem.first;
				num = elem.second;

				if (num < obj->children.size() && obj->children[num]->command == "else") {
					++num;
					++stack.back().second;
				}
			}
			continue;
		}

		if (child->command == "return") {
			while (obj->command != "label") {
				stack.pop_back();
				if (stack.empty()) {
					Utils::outMsg("Scenario::execute", "return outside label\n" + child->getPlace());
					break;
				}

				elem = stack.back();
				obj = elem.first;
				num = elem.second;
			}

			stack.pop_back();
			if (stack.empty()) {
				obj = nullptr;
				continue;
			}

			elem = stack.back();
			obj = elem.first;
			num = elem.second;

			continue;
		}

		if (child->command == "with") {
			const std::string &effectName = child->params;
			const std::string code = "screen.set_effect(" + effectName + ")";
			PyUtils::exec(child->getFileName(), child->getNumLine(), code);
			continue;
		}

		if (child->command != "pass") {
			Utils::outMsg("Scenario::execute",
			              "Unexpected command <" + child->command + ">\n" +
			              child->getPlace());
		}
	}


	if (!Game::hasLabel("start")) {
		while (GV::inGame) {
			Utils::sleep(Game::getFrameTime());
			checkToSaveStack();
		}
	}

	if (GV::inGame) {
		Game::startMod("main_menu");
	}
}
