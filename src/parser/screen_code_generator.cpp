#include "screen_code_generator.h"

#include <map>

#include "gui/screen/screen.h"

#include "media/py_utils.h"

#include "parser/screen_update_funcs.h"

#include "utils/algo.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"



static void initConsts(Node *node);
static void initIsEnd(Node *node);
static void initNums(Node *node);
static std::string initCode(Node *node, const std::string& index = "");


static std::map<const Node*, std::string> screenCodes;


void initScreen(Node *node) {
	if (screenCodes.find(node) != screenCodes.end()) return;

	initConsts(node);
	initIsEnd(node);
	initNums(node);

	std::string screenCode = initCode(node);
	screenCodes[node] = screenCode;

	ScreenUpdateFuncs::initNodeFuncs(node);
}


std::string ScreenCodeGenerator::get(Node *screenNode) {
	auto it = screenCodes.find(screenNode);
	if (it != screenCodes.end()) return it->second;

	initScreen(screenNode);
	return screenCodes[screenNode];
}

void ScreenCodeGenerator::clear() {
	screenCodes.clear();
}


static std::vector<std::string> screensInInit;

static void initConsts(Node *node) {
	const std::string &command = node->command;
	const std::string &params = node->params;

	node->countPropsToCalc = 0;

	if (node->isScreenProp) {
		if (node->isScreenEvent) {
			node->parent->withScreenEvent = true;
		}else {
			node->isScreenConst = command == "has" || PyUtils::isConstExpr(params);
		}
		return;
	}
	if (command == "$" || command == "python") return;

	ScopeExit se(nullptr);
	if (command == "screen") {
		screensInInit.push_back(params);
		se.func = [&]() {
			screensInInit.pop_back();
		};
	}

	if (command == "use") {
		Node *screenNode = Screen::getDeclared(node->params);

		if (screenNode) {
			if (Algo::in(params, screensInInit)) {
				Utils::outMsg("ScreenCodeGenerator::initConsts",
				              "Using screens with recursion: " + String::join(screensInInit, " -> ") + " -> " + params);
				node->command = "pass";
			}else {
				initScreen(screenNode);
				node->isScreenConst = screenNode->isScreenConst;
				node->countPropsToCalc = screenNode->countPropsToCalc;
			}
		}else {
			Utils::outMsg("ScreenCodeGenerator::initConsts",
			              "Screen with name <" + node->params + "> not found\n" +
			              node->getPlace());
			node->command = "pass";
		}
		return;
	}

	for (Node *child : node->children) {
		initConsts(child);

		if (child->isScreenProp && !child->isScreenConst && !child->isScreenEvent) {
			++node->countPropsToCalc;
		}
	}


	if (command == "button" || command == "textbutton" || command == "hotspot") {
		node->withScreenEvent = true;
		return;
	}

	if (command == "if" || command == "elif" || command == "else" ||
	    command == "for" || command == "while") return;

	for (const Node *child : node->children) {
		if (!child->isScreenConst) return;
	}
	node->isScreenConst = true;
}

static void initIsEnd(Node *node) {
	if (node->isScreenProp) return;

	const std::string &command = node->command;

	if (command == "$" || command == "python") return;
	if (command == "use") return;

	for (Node *child : node->children) {
		initIsEnd(child);
	}
	if (command == "screen") return;

	if (command == "if" || command == "elif" || command == "else" ||
	    command == "for" || command == "while") return;

	for (const Node *child : node->children) {
		if (!child->isScreenProp && !child->isScreenConst) return;
	}
	node->isScreenEnd = true;
}

static void initNums(Node *node) {
	uint32_t i = 0;
	for (Node *child : node->children) {
		if (child->isScreenConst || child->isScreenEvent ||
		    child->command == "$" || child->command == "python") continue;

		child->screenNum = i++;
		initNums(child);
	}
}



static uint32_t maxScreenChild(Node *node) {
	for (size_t i = node->children.size() - 1; i != size_t(-1); --i) {
		Node *child = node->children[i];
		if (child->screenNum != uint32_t(-1)) {
			return child->screenNum;
		}
	}

	return uint32_t(-1);
}


static void initChildCode(Node *child, std::string &res, const std::string &indent, const std::string &index) {
	const std::string &childCommand = child->command;

	std::string childRes = initCode(child, index);
	if (childRes.empty()) return;

	std::vector<std::string> code = String::split(childRes, "\n");

	if (child->isScreenProp || child->isScreenEnd) {
		if (childCommand != "continue" && childCommand != "break") {
			res += indent + "_SL_last[" + index + "] = ";
		}else {
			res += indent;
		}

		for (size_t i = 0; i < code.size(); ++i) {
			if (i) {
				res += indent;
			}
			res += code[i];
			if (i != code.size() - 1) {
				res += '\n';
			}
		}

		if (child->withScreenEvent) {
			res += "\n" +
			       indent + "_SL_check_events()";
		}
		res += '\n';

		return;
	}


	if (childCommand != "elif" && childCommand != "else") {
		if (childCommand != "$" && childCommand != "python") {
			res += indent + "_SL_stack.append(_SL_last)\n";
		}
	}

	for (const std::string &line : code) {
		res += indent + line + '\n';
	}

	bool hasEnd = true;
	if (childCommand == "if" || childCommand == "elif" ||
	    childCommand == "for" || childCommand == "while") // child may be not end-[condition/cycle]
	{
		if (child->childNum + 1 < child->parent->children.size()) { // and nextChild
			Node *nextChild = child->parent->children[child->childNum + 1];
			if (nextChild->command == "elif" || nextChild->command == "else") { // may be end-[condition/cycle]
				hasEnd = false;
			}
		}
	}
	if (hasEnd) {
		if (childCommand != "$" && childCommand != "python") {
			res += indent + "_SL_last = _SL_stack.pop()\n";
		}
	}
}


static std::string initCycleCode(Node *node) {
	std::string res;
	std::string id = std::to_string(node->id);
	std::string name = "_SL_counter" + id;
	std::string idLen = "_SL_len" + id;
	std::string indent = "    ";

	const std::string &command = node->command;
	const std::string &params = node->params;

	uint32_t count = maxScreenChild(node) + 1;

	PyUtils::exec(node->getFileName(), node->getNumLine(), name + " = 0");

	if (node->childNum + 1 < node->parent->children.size()) {
		Node *next = node->parent->children[node->childNum + 1];
		if (next->command == "else") {
			res += "_SL_break" + id + " = False\n";
		}
	}

	res +=
	    "_SL_last[" + std::to_string(node->screenNum) + "] = _SL_last = [None] * " + name + "\n" +
	    idLen + " = " + name + "\n" +
	    name + " = 0\n"
	    "\n" +
	    command + ' ' + params + ": # " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n" +
	    indent + "\n";

	if (count) {
		res +=
		    indent + "if " + name + " == " + idLen + ":\n" +
		    indent + "    _SL_last += [None] * " + std::to_string(32 * count) + "\n" +
		    indent + "    " + idLen + " += " + std::to_string(32 * count) + "\n" +
		    indent + "\n";
	}


	bool empty = true;
	for (Node *child : node->children) {
		if (child->isScreenConst && child->command != "continue" && child->command != "break") continue;
		if (child->isScreenEvent) continue;

		std::string index = name;
		if (child->screenNum) {
			index += " + " + std::to_string(child->screenNum);
		}
		initChildCode(child, res, indent, index);

		res += indent + '\n';
		empty = false;
	}
	if (empty) {
		res += indent + "pass\n";
	}

	if (count) {
		res += indent + name + " += " + std::to_string(count) + "\n";
	}

	return res;
}

static std::string initUseCode(Node *node, const std::string &index) {
	Node *screenNode = Screen::getDeclared(node->params);
	if (!screenNode) return "";
	if (screenNode->vars.size() < node->vars.size()) {
		Utils::outMsg("ScreenCodeGenerator::initUseCode",
		              "Maximum " + std::to_string(screenNode->vars.size()) + " args expected"
		              "for screen <" + node->params + ">, got " + std::to_string(node->vars.size()) + "\n\n" +
		              node->getPlace());
	}

	std::string inner = initCode(screenNode, index);
	std::string res =
	        "_SL_screen_vars_stack.append(screen)\n"
	        "screen = Object()\n";


	std::vector<std::string> argsWasSet;
	argsWasSet.reserve(node->vars.size());

	for (size_t i = 0; i < node->vars.size(); ++i) {
		auto &[varName, varCode] = node->vars[i];

		const std::string *namePtr = &varName;
		if (namePtr->empty() && i < screenNode->vars.size()) {
			namePtr = &screenNode->vars[i].first;
		}
		const std::string &name = *namePtr;

		if (!name.empty()) {
			if (std::find_if(
			        screenNode->vars.cbegin(),
			        screenNode->vars.cend(),
			        [&name] (const std::pair<std::string, std::string> &p) -> bool { return p.first == name; }
			) == screenNode->vars.cend())
			{
				Utils::outMsg("ScreenCodeGenerator::initUseCode",
				              "No param <" + name + "> in screen <" + node->params + ">\n\n" +
				              node->getPlace());
			}else {
				argsWasSet.push_back(name);
				res += "screen." + name + " = " + varCode + "\n";
			}
		}
	}

	for (size_t i = 0; i < screenNode->vars.size(); ++i) {
		auto &[varName, varCode] = screenNode->vars[i];
		if (std::find(argsWasSet.cbegin(), argsWasSet.cend(), varName) != argsWasSet.cend()) continue;

		if (varCode.empty()) {
			Utils::outMsg("ScreenCodeGenerator::initUseCode",
			              "No value for param <" + varName + "> in screen <" + node->params + ">\n\n" +
			              node->getPlace());
		}else {
			res += "screen." + varName + " = " + varCode + "\n";
		}
	}

	res += "\n" +
	       inner + "\n\n" +
	       "screen = _SL_screen_vars_stack.pop()";
	return res;
}

static std::string initCode(Node *node, const std::string& index) {
	const std::string &command = node->command;
	const std::string &params = node->params;

	std::string res;

	if (command == "use") {
		res = initUseCode(node, index);
		return res;
	}

	if (command == "continue" || command == "break") {
		Node *t = node->parent;
		int extraDepth = 0;
		while (t->command != "for" && t->command != "while") {
			t = t->parent;
			++extraDepth;
		}
		std::string id = std::to_string(t->id);

		res = "# " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n";

		if (command == "break" &&
		    t->childNum + 1 < t->parent->children.size() &&
		    t->parent->children[t->childNum + 1]->command == "else")
		{
			res += "_SL_break" + id + " = True\n";
		}
		std::string removeStackTop;
		if (extraDepth) {
			for (int i = 0; i < extraDepth - 1; ++i) {
				removeStackTop += "_SL_stack.pop()\n";
			}
			removeStackTop += "_SL_last = _SL_stack.pop()\n";
		}

		res += "_SL_counter" + id + " += " + std::to_string(maxScreenChild(t) + 1) + "\n" +
		       removeStackTop +
		       command + "\n";
		return res;
	}

	bool isMainScreen = command == "screen" && index.empty();
	if ((node->isScreenConst && !isMainScreen) || node->isScreenEvent) return "";

	if (node->isScreenProp) {
		res = String::strip(params);
		res += "# " + std::to_string(node->getNumLine()) + " " + node->getFileName();
		return res;
	}

	if (command == "$" || command == "python") {
		size_t startLine = node->getNumLine() + (command == "python");

		const std::vector<std::string> code = String::split(params, "\n");
		for (size_t i = 0; i < code.size(); ++i) {
			if (code[i].empty()) continue;

			res += code[i] + " # " + std::to_string(startLine + i) + " " + node->getFileName();
			if (i != code.size() - 1) {
				res += "\n";
			}
		}

		return res;
	}

	if (command == "for" || command == "while") return initCycleCode(node);



	if (node->isScreenEnd) {
		res = "(\n";
		for (Node *child : node->children) {
			if (child->isScreenConst || child->isScreenEvent) continue;

			std::string childRes = initCode(child);
			childRes.insert(String::firstNotInQuotes(childRes, '#'), ", ");

			res += "    " + childRes + "\n";
		}

		if (res.size() != 2) {
			res += ")";
		}else {
			res[1] = ')';
		}

		return res;
	}


	std::string indent;

	if (command == "screen") {
		if (index.empty()) {
			res = "_SL_stack = []\n"
			      "_SL_screen_vars_stack = []\n"
			      "screen = screen_vars['" + params + "']\n\n";
		}
	}else
	if (command == "if" || command == "elif" || command == "else") {
		indent = "    ";

		if (command == "else" && !String::endsWith(node->parent->children[node->childNum - 1]->command, "if")) {//prev is for/while
			res += "if not _SL_break" + std::to_string(node->parent->children[node->childNum - 1]->id) + ":";
			res +=   " # " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n";
			res += indent + "_SL_last = _SL_stack[-1]\n";
		}else {
			res = command + " " + params + ": # " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n";
		}
	}

	res += indent;
	if (command == "screen" && index.empty()) {
		res += "_SL_" + params + " = ";
	}else {
		res += "_SL_last[" + index + "] = ";
	}
	res += "_SL_last = [\n";
	size_t bracketBegin = res.size() - 2;

	uint32_t lastPropNum = uint32_t(-1);
	uint32_t maxChildNum = maxScreenChild(node);

	//props before first obj
	for (Node *child : node->children) {
		if (child->isScreenConst) continue;
		if (!child->isScreenProp) break;

		std::string childRes = initCode(child);
		childRes.insert(String::firstNotInQuotes(childRes, '#'), ", ");

		res += indent + "    " + childRes + "\n";
		lastPropNum = child->screenNum;
	}

	if (maxChildNum != uint32_t(-1)) {
		res += String::repeat(indent + "    None,\n", maxChildNum - lastPropNum);
		res += indent + "]";
	}else {
		res.back() = ']';
	}

	bool empty = false;
	if (lastPropNum == maxChildNum) {
		res[bracketBegin] = '(';
		res.back() = ')';

		empty = true;
		for (const Node *child : node->children) {
			if (child->command == "$" || child->command == "python" ||
			    child->command == "continue" || child->command == "break")
			{
				empty = false;
				break;
			}
		}
	}

	if (!empty) {
		res += "\n" +
		       indent + "\n";

		for (Node *child : node->children) {
			if (child->isScreenConst && child->command != "continue" && child->command != "break") continue;
			if (child->isScreenEvent) continue;

			if (child->screenNum <= lastPropNum && lastPropNum != uint32_t(-1)) continue;

			initChildCode(child, res, indent, std::to_string(child->screenNum));

			if (child->screenNum != maxChildNum) {
				res += indent + '\n';
			}
		}
	}


	if (isMainScreen) {
		res += "\n"
		       "_SL_check_events()\n";
	}

	return res;
}
