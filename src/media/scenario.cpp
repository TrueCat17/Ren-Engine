#include "scenario.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <vector>


#include "config.h"
#include "gv.h"
#include "logger.h"

#include "gui/screen/screen.h"

#include "media/audio_manager.h"
#include "media/py_utils.h"
#include "media/sprite.h"
#include "media/translation.h"

#include "parser/node.h"

#include "utils/algo.h"
#include "utils/game.h"
#include "utils/mouse.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"



bool Scenario::initing;


static std::map<std::string, Node*> declaredLabels;

static void declareLabel(Node *labelNode) {
	declaredLabels[labelNode->params] = labelNode;
}
static Node* getLabel(const std::string &name, bool outError = true) {
	auto it = declaredLabels.find(name);
	if (it != declaredLabels.end()) return it->second;

	if (outError) {
		Utils::outError("Scenario::getLabel", "Label <%> not found", name);
	}
	return nullptr;
}
static void markLabel(const std::string &fileName, uint32_t numLine, const std::string &label) {
	PyUtils::execWithSetTmp(fileName, numLine,
	                        "persistent._seen_labels[get_current_mod()][tmp] = True", label);
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
		return;
	}

	std::ifstream is(loadPath + "/stack");

	Node *prevNode = nullptr;
	size_t num = 0;

	auto check = [&](bool zeroIndexIsError) {
		if (!prevNode) return true;
		if (num <= prevNode->children.size() && (num || !zeroIndexIsError)) return true;

		if (num) {
			Utils::outError("Scenario::restoreStack",
			                "Node have only % children, need #%\n%",
			                prevNode->children.size(), num, prevNode->getPlace());
		}else {
			Utils::outError("Scenario::restoreStack",
			                "Unexpected stack element after last element <%>\n%",
			                prevNode->command, prevNode->getPlace());
		}
		return false;
	};
	auto getChild = [&](void) -> Node* {
		if (check(true)) {
			return prevNode->children[num - 1];
		}
		return nullptr;
	};

	//check last stack element in the end
	ScopeExit se([&]() {
		if (!check(false)) {
			auto &elem = stack.back();
			elem.second = elem.first->children.size();
		}
	});

	while (!is.eof()) {
		std::string tmp;
		std::getline(is, tmp);
		if (tmp.empty()) break;

		std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 2) {
			Utils::outError("Scenario::restoreStack", "In string <%> expected 2 args", tmp);
			return;
		}

		std::string first = tmpVec[0];
		std::string second = tmpVec[1];

		Node *node;

		if (String::startsWith(first, "label:") || first == "menu") {
			if (first != "menu") {
				std::string labelName = first.substr(strlen("label:"));
				node = getLabel(labelName);
			}else {
				node = getChild();
			}
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

		node = getChild();
		if (!node) return;

		if (node->command != first) {
			Utils::outError("Scenario::restoreStack",
			                "Expected command <%>, got <%>\n%",
			                first, node->command, node->getPlace());
			return;
		}

		num = size_t(String::toInt(second));

		stack.push_back({node, num});
		prevNode = node;
	}
}

static bool needToSaveStack = false;
static std::vector<std::pair<std::string, std::string>> stackToSave;
void Scenario::saveStack(const std::string &path) {
	needToSaveStack = true;
	while (needToSaveStack && GV::inGame) {
		Utils::sleep(Game::getFrameTime());
	}

	std::ofstream stackFile(path, std::ios::binary);
	if (stackToSave.empty()) {
		stackFile << '\n';
	}else {
		for (auto p : stackToSave) {
			stackFile << p.first << ' ' << p.second << '\n';
		}
	}
}
static void checkToSaveStack() {
	if (!needToSaveStack) return;

	stackToSave.clear();
	stackToSave.reserve(stack.size());

	for (const auto &[node, num] : stack) {
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
		PyUtils::execWithSetTmp("CPP_EMBED: scenario.cpp", __LINE__,
		                        "pickling.load_global_vars(tmp + '/py_globals')", loadPath);

		startScreensVec = Game::loadInfo(loadPath);
		Screen::clearScreensToShow();
	}else {
		PyUtils::callInPythonThread([&]() {
			PyObject *startScreens = PyDict_GetItemString(PyUtils::global, "start_screens");
			if (!startScreens) {
				Utils::outMsg("Scenario::restoreScreens", "start_screens is not defined");
				return;
			}

			if (!PyList_CheckExact(startScreens)) {
				Utils::outMsg("Scenario::restoreScreens", "type(start_screens) is not list");
				return;
			}

			size_t len = size_t(Py_SIZE(startScreens));
			for (size_t i = 0; i < len; ++i) {
				PyObject *elem = PyList_GET_ITEM(startScreens, i);
				if (!PyUnicode_CheckExact(elem)) {
					Utils::outError("Scenario::restoreScreens",
					                "type(start_screens[%]) is not str", i);
					continue;
				}

				std::string name = PyUtils::objToStr(elem);
				startScreensVec.push_back(name);
			}
		});
	}

	for (const std::string &screenName : startScreensVec) {
		if (!screenName.empty()) {
			Screen::addToShow(screenName, "CPP_EMBED: scenario.cpp", __LINE__);
		}
	}
}


static void makeStyle(const Node *child) {
	std::string name, parent;
	std::vector<std::string> words = String::split(child->params, " ");
	if (words.size() == 1) {
		name = child->params;
		parent = "default";
	}else {
		if (words.size() != 3 || words[1] != "is") {
			Utils::outError("Scenatio::execute",
			                "Expected 'style name [is parent]', got: style %\n%",
			                child->params, child->getPlace());
			return;
		}
		name = words[0];
		parent = words[2];
	}

	if (!PyUtils::isIdentifier(name)) {
		Utils::outError("Scenatio::execute",
		                "Invalid style name <%>\n%",
		                name, child->getPlace());
		return;
	}
	if (!PyUtils::isIdentifier(parent)) {
		Utils::outError("Scenatio::execute",
		                "Invalid style name <%>\n%",
		                parent, child->getPlace());
		return;
	}

	if (name != "default") {
		if (PyUtils::exec(child->getFileName(), child->getNumLine(), "style." + parent + " is None", true) == "True") {
			Utils::outError("Scenario::execute",
			                "Style <%> does not exist\n%",
			                parent, child->getPlace());
		}
	}

	std::string code;
	code += "if not style." + name + ":\n";
	code += "    style." + name + " = Style(style." + parent + ")\n";
	if (words.size() != 1) {
		code += "else:\n";
		code += "    style." + name + ".properties = style." + parent + "\n";
	}
	for (const Node* prop : child->children) {
		code += "style." + name + "." + prop->command + " = " + prop->params + "\n";
	}

	PyUtils::exec(child->getFileName(), child->getNumLine(), code);
}


static void displayCommandProcessing(const Node *child) {
	std::string funcName;
	if (child->command == "scene") {
		funcName = "sprites.set_scene";
	}else {
		if (String::startsWith(child->params, "screen ")) {
			const std::string screenName = child->params.substr(strlen("screen "));
			if (child->command == "show") {
				Screen::addToShow(screenName, child->getFileName(), child->getNumLine());
			}else {
				Screen::addToHide(screenName, child->getFileName(), child->getNumLine());
			}
			return;
		}

		funcName = "sprites." + child->command;
	}

	const std::vector<std::string> args = Algo::getArgs(child->params);
	if (args.empty() && child->command != "scene") {
		Utils::outError("Scenario::execute",
		                "Invalid command <% %>\n%",
		                child->command, child->params, child->getPlace());
		return;
	}

	PyUtils::callInPythonThread([&]() {
		PyObject *list = PyList_New(Py_ssize_t(args.size()));

		for (size_t i = 0; i < args.size(); ++i) {
			const std::string &arg = args[i];
			PyObject *str = PyUnicode_FromStringAndSize(arg.c_str(), Py_ssize_t(arg.size()));
			PyList_SET_ITEM(list, Py_ssize_t(i), str);
		}

		PyDict_SetItemString(PyUtils::global, "tmp", list);
		Py_DECREF(list);

		std::string argsStr = "tmp";

		if (child->command != "hide") {
			int count = String::toInt(Config::get("count_preload_commands"));
			Node::preloadImages(child->parent, child->childNum + 1, size_t(std::max(count, 0)));

			argsStr += ", last_show_at";

			PyObject *actions = child->getPyChildren();
			PyDict_SetItemString(PyUtils::global, "last_show_at", actions);
			Py_DECREF(actions);
		}

		PyUtils::exec(child->getFileName(), child->getNumLine(), funcName + "(" + argsStr + ")");
	});
}



static std::string jumpNextLabel;
static bool jumpNextIsCall;
static std::string jumpNextFileName;
static uint32_t jumpNextNumLine;

void Scenario::jumpNext(const std::string &label, bool isCall, const std::string &fileName, uint32_t numLine) {
	jumpNextLabel = label;
	jumpNextIsCall = isCall;
	jumpNextFileName = fileName;
	jumpNextNumLine = numLine;
}


void Scenario::execute(const std::string &loadPath) {
	double initingStartTime = Utils::getTimer();
	initing = true;
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
			markLabel(jumpNextFileName, jumpNextNumLine, label);

			obj = labelNode;
			num = 0;
			stack.push_back({obj, num});
			continue;
		}

		if (!GV::inGame || !obj) break;


		if (!initing && !inWithBlock) {
			checkToSaveStack();

			static const std::string code = "can_exec_next_command()";
			while (true) {
				if (!GV::inGame) return;
				if (PyUtils::exec("CPP_EMBED: scenario.cpp", __LINE__, code, true) == "True") break;
				if (!jumpNextLabel.empty()) break;

				Utils::sleep(Game::getFrameTime());
				checkToSaveStack();
			}

			if (!jumpNextLabel.empty()) {
				PyUtils::exec("CPP_EMBED: scenario.cpp", __LINE__, "skip_exec_current_command(full = True)");
				continue;
			}
		}


		if (num == obj->children.size()) {
			if (obj->command == "init" || obj->command == "init python") {
				if (obj->command == "init python") {
					PyUtils::exec(obj->getFileName(), obj->getNumLine() + 1, obj->params);
				}
				PyUtils::exec(obj->getFileName(), obj->getNumLine(), "default_decl_at = ()");

				++initNum;
				if (initNum == initBlocks.size()) {
					Translation::enable();
					PyUtils::exec("CPP_EMBED: scenario.cpp", __LINE__, "_choose_lang()");
					PyUtils::exec("CPP_EMBED: scenario.cpp", __LINE__, "signals.send('inited')");

					restoreScreens(loadPath);
					Logger::logEvent("Mod Initing (" + std::to_string(initBlocks.size()) + " blocks)",
					                 Utils::getTimer() - initingStartTime, true);

					if (Game::hasLabel("start")) {
						Node *start = getLabel("start");
						markLabel(start->getFileName(), start->getNumLine(), "start");
					}

					initing = false;
					GV::beforeFirstFrame = false;
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
				while (num < obj->children.size() &&
				       String::startsWith(obj->children[num]->command, "el")) //<elif>, <else>
				{
					++num;
					++stack.back().second;
				}
			}

			continue;
		}


		if (obj->command == "menu") {
			const std::string resStr = PyUtils::exec(obj->getFileName(), obj->getNumLine(),
			                                         "int(choice_menu_result)", true);
			int res = String::toInt(resStr);

			if (res < 0 || res >= int(obj->children.size())) {
				Utils::outError("Scenario::execute",
				                "choice_menu_result = %, min = 0, max = %\n%",
				                resStr, obj->children.size(), obj->getPlace());
				res = 0;
			}

			stack.back().second = size_t(res) + 1;

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
			bool screenIsShowed = Screen::getMain("choice_menu");
			if (!screenIsShowed) {
				PyUtils::callInPythonThread([&]() {
					PyObject *list = PyList_New(Py_ssize_t(child->children.size()));

					for (size_t i = 0; i < child->children.size(); ++i) {
						const Node *node = child->children[i];
						PyObject *variant = PyUtils::execRetObj(node->getFileName(), node->getNumLine(), node->params);
						if (!variant) {
							variant = Py_NewRef(Py_None);
						}
						PyList_SET_ITEM(list, Py_ssize_t(i), variant);
					}

					PyDict_SetItemString(PyUtils::global, "choice_menu_variants", list);
					Py_DECREF(list);

					std::string code = "renpy.call_screen('choice_menu', 'choice_menu_result')";
					PyUtils::exec(child->getFileName(), child->getNumLine(), code);
				});
			}

			obj = child;
			num = 0;

			stack.push_back({obj, num});
			continue;
		}

		if (child->command == "image") {
			Sprite::registerImage(child);
			continue;
		}

		if (child->command == "scene" || child->command == "show" || child->command == "hide") {
			displayCommandProcessing(child);
			continue;
		}

		if (child->command == "style") {
			makeStyle(child);
			continue;
		}

		if (child->command == "transform") {
			if (!PyUtils::isIdentifier(child->params)) {
				Utils::outError("Scenatio::execute",
				                "Invalid transform name <%>\n%",
				                child->params, child->getPlace());
				continue;
			}

			PyUtils::callInPythonThread([&]() {
				PyObject *actions = child->getPyChildren();
				PyDict_SetItemString(PyUtils::global, "tmp", actions);
				Py_DECREF(actions);

				PyUtils::exec(child->getFileName(), child->getNumLine(),
				              child->params + " = SpriteAnimation(tmp)");
			});
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
			const uint32_t numLine = child->getNumLine() + (child->command != "$");

			PyUtils::exec(child->getFileName(), numLine, child->params);
			Utils::sleep(1e-3 / 20);
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
			bool whoDefined = false;
			std::string who;
			std::string what;

			if (child->params[0] != '"' && child->params[0] != '\'') {//Is character defined?
				size_t space = child->params.find(' ');
				if (space != size_t(-1)) {
					size_t startText = child->params.find_first_not_of(' ', space);
					if (startText != size_t(-1)) {
						what = child->params.substr(startText);
					}
				}

				who = child->params.substr(0, space);
				whoDefined = true;
			}else {
				who = "narrator";
				what = child->params;
			}


			std::string tmp = (whoDefined ? who + " " : "") + what + "\r\n";
			std::string md5 = Utils::md5(tmp);

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
				              "renpy.say(" + who + ", " + what + ")");
			}
			continue;
		}

		if (child->command == "play") {
			AudioManager::playWithParsing(child->params, child->getFileName(), child->getNumLine());
			continue;
		}

		if (child->command == "queue") {
			AudioManager::queueWithParsing(child->params, child->getFileName(), child->getNumLine());
			continue;
		}

		if (child->command == "stop") {
			AudioManager::stopWithParsing(child->params, child->getFileName(), child->getNumLine());
			continue;
		}


		if (child->command == "continue" || child->command == "break") {
			size_t i = stack.size() - 1;
			while (i && stack[i].first->command != "while") {
				--i;
			}
			if (!i) {
				Utils::outError("Scenario::execute",
				                "% outside loop\n%",
				                child->command, child->getPlace());
				continue;
			}

			stack.erase(stack.begin() + long(i + 1), stack.end());

			if (child->command == "continue") {
				elem = stack.back();
				obj = elem.first;
				num = elem.second = obj->children.size();
			}else {
				stack.pop_back();
				if (stack.empty()) {//unreachable (top level - <label>, for example, not <while>), but...
					Utils::outError("Scenario::execute",
					                "% outside loop\n%",
					                child->command, child->getPlace());
					break;
				}

				elem = stack.back();
				obj = elem.first;
				num = elem.second;

				if (num < obj->children.size() && obj->children[num]->command == "else") {
					++num;
					++elem.second;
				}
			}
			continue;
		}

		if (child->command == "return") {
			size_t i = stack.size() - 1;
			while (i != size_t(-1) && stack[i].first->command != "label") {
				--i;
			}
			if (i == size_t(-1)) {
				Utils::outError("Scenario::execute",
				                "return outside label\n%", child->getPlace());
				continue;
			}

			stack.erase(stack.begin() + long(i), stack.end());

			if (stack.empty()) break;

			elem = stack.back();
			obj = elem.first;
			num = elem.second;

			continue;
		}

		if (child->command == "with") {
			const std::string &effectName = child->params;
			const std::string code = "sprites.just_effect(" + effectName + ")";
			PyUtils::exec(child->getFileName(), child->getNumLine(), code);
			continue;
		}

		if (child->command != "pass") {
			Utils::outError("Scenario::execute",
			                "Unexpected command <%>\n%",
			                child->command, child->getPlace());
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
