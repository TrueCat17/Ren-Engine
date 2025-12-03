#include "screen_code_generator.h"

#include <algorithm>
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
static std::vector<std::string> initCode(Node *node, const std::string& index = "");


static std::map<const Node*, std::string> screenCodes;


static void initScreen(Node *node) {
	if (screenCodes.find(node) != screenCodes.end()) return;

	initConsts(node);
	initIsEnd(node);
	initNums(node);

	const std::vector<std::string> screenCodeStrs = initCode(node);

	std::string screenCode;
	for (size_t i = 0; i < screenCodeStrs.size(); ++i) {
		const std::string &s = screenCodeStrs[i];
		screenCode += s;

		//(\n) -> ()
		if (!s.empty() && s.back() == '(' && i != screenCodeStrs.size() - 1) {
			const std::string &next = screenCodeStrs[i + 1];
			if (!next.empty() && next.find_first_not_of(' ') == next.size() - 1 && next.back() == ')') {
				screenCode += ")";
				++i;
			}
		}

		screenCode.push_back('\n');
	}
	while (!screenCode.empty() && screenCode.back() == '\n') {
		screenCode.pop_back();
	}

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
	if (command == "continue" || command == "break") return;

	ScopeExit se([&]() { screensInInit.pop_back(); });
	se.enable = false;
	if (command == "screen") {
		screensInInit.push_back(params);
		se.enable = true;
	}

	if (command == "use") {
		bool ok = false;
		Node *screenNode = Screen::getDeclared(node->params);

		if (screenNode) {
			if (Algo::in(params, screensInInit)) {
				Utils::outError("ScreenCodeGenerator::initConsts",
				                "Using screens with recursion: % -> %",
				                String::join(screensInInit, " -> "), params);
			}else {
				ok = true;
				initScreen(screenNode);
				node->isScreenConst = screenNode->isScreenConst;
				node->countPropsToCalc = screenNode->countPropsToCalc;
			}
		}else {
			Utils::outError("ScreenCodeGenerator::initConsts",
			                "Screen with name <%> not found\n%",
			                node->params, node->getPlace());
		}
		if (ok) return;

		node->command = "pass";
	}

	bool isCycle = command == "for" || command == "while";
	for (Node *child : node->children) {
		initConsts(child);
		if (isCycle) {
			//unable to count iterations for an empty (constant children only) cycle
			// => cycle children must be not const
			child->isScreenConst = false;
		}

		if (child->isScreenProp && !child->isScreenConst && !child->isScreenEvent) {
			++node->countPropsToCalc;
		}else
		if (child->command == "continue" || child->command == "break") {
			++node->countPropsToCalc;
		}
	}

	if (isCycle || command == "if" || command == "elif" || command == "else") return;

	if (command == "button" || command == "textbutton" || command == "hotspot") {
		node->withScreenEvent = true;
		return;
	}

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
		if (child->isScreenConst || child->isScreenEvent) continue;
		if (child->command == "$" || child->command == "python") continue;

		child->screenNum = i++;
		initNums(child);
	}
}



//last index = -1;
//prelast = -2;
//...
static std::vector<std::pair<int, Node*>> getNumberedActiveChildren(Node *node, bool *eachChildIsEnd) {
	auto withObject = [](const std::string &command) {
		return command != "$" && command != "python";
	};

	std::vector<std::pair<int, Node*>> res;
	res.reserve(node->children.size());

	if (eachChildIsEnd) {
		*eachChildIsEnd = true;
	}

	size_t objects = 0;
	for (Node *child : node->children) {
		if (child->isScreenConst || child->isScreenEvent) continue;

		res.push_back({1234567, child});

		if (eachChildIsEnd && !child->isScreenEnd) {
			*eachChildIsEnd = false;
		}

		if (withObject(child->command)) {
			++objects;
		}
	}

	size_t counted = 0;
	for (auto &pair : res) {
		if (withObject(pair.second->command)) {
			pair.first = int(counted++ - objects);
		}
	}

	return res;
}


static void initChildCode(Node *child, std::vector<std::string> &res, const std::string &indent, const std::string &index) {
	const std::string &childCommand = child->command;

	const std::vector<std::string> code = initCode(child, index);
	if (code.empty()) return;

	if (child->isScreenProp || child->isScreenEnd) {
		res.push_back(indent);
		if (childCommand != "continue" && childCommand != "break" && !index.empty()) {
			res.back() += "_SL_last[" + index + "] = ";
		}

		for (size_t i = 0; i < code.size(); ++i) {
			if (i) {
				res.back() += indent;
			}
			res.back() += code[i];
			if (i != code.size() - 1) {
				res.push_back({});
			}
		}

		if (child->withScreenEvent) {
			res.push_back(indent + "_SL_check_events()");
		}

		return;
	}


	if (childCommand != "elif" && childCommand != "else") {
		if (childCommand != "$" && childCommand != "python") {
			res.push_back(indent + "_SL_stack.append(_SL_last)");
		}
	}

	for (const std::string &line : code) {
		res.push_back(indent + line);
	}

	if (childCommand == "$" || childCommand == "python") return;

	if (childCommand == "if" || childCommand == "elif" ||
	    childCommand == "for" || childCommand == "while") // child may be not end of condition/cycle
	{
		if (child->childNum + 1 < child->parent->children.size()) { // and nextChild
			Node *nextChild = child->parent->children[child->childNum + 1];
			if (nextChild->command == "elif" || nextChild->command == "else") { // may be end of condition/cycle
				return;
			}
		}
	}

	res.push_back(indent + "_SL_last = _SL_stack.pop()");
}


static std::vector<std::string> initCycleCode(Node *node, const std::string &index) {
	std::vector<std::string> res;

	const std::string indent = "    ";

	const std::string &command = node->command;
	const std::string &params = node->params;

	if (node->childNum + 1 < node->parent->children.size()) {
		Node *next = node->parent->children[node->childNum + 1];
		if (next->command == "else") {
			res.push_back("_SL_break" + std::to_string(node->id) + " = False");
		}
	}

	res.push_back("_SL_last[" + index + "] = _SL_last = []");
	res.push_back({});
	res.push_back(command + ' ' + params);
	res.back() += ": # _SL_REAL|" + node->getFileName() + "|" + std::to_string(node->getNumLine());
	res.push_back(indent);


	bool eachChildIsEnd;
	const std::vector<std::pair<int, Node*>> activeChildren = getNumberedActiveChildren(node, &eachChildIsEnd);

	if (!eachChildIsEnd) {
		for (const auto &[index, child] : activeChildren) {
			if (index < 0) {
				res.push_back(indent + "_SL_last.append(None)");
			}
		}
		res.push_back(indent);
	}

	std::string checkEventsStr = indent + "_SL_check_events()";

	for (size_t i = 0; i < activeChildren.size(); ++i) {
		const auto &[index, child] = activeChildren[i];

		std::string indexStr;
		if (!eachChildIsEnd) {
			indexStr = std::to_string(index);
		}else {
			res.push_back(indent + "_SL_last.append(");
		}

		initChildCode(child, res, indent, indexStr);

		if (eachChildIsEnd) {
			size_t i = res.size();
			while (--i) {
				if (res[i] == checkEventsStr) break;
			}

			if (i) {
				res.insert(res.begin() + long(i), indent + ")");
			}else {
				res.push_back(indent + ")");
			}
		}

		res.push_back(indent);
	}

	if (activeChildren.empty()) {
		res.push_back(indent + "pass");
	}

	return res;
}

static std::vector<std::string> initUseCode(Node *node, const std::string &index) {
	std::vector<std::string> res;

	Node *screenNode = Screen::getDeclared(node->params);
	if (!screenNode) return res;

	auto &nodeVars = node->getScreenVars();
	auto &screenNodeVars = screenNode->getScreenVars();

	if (screenNodeVars.size() < nodeVars.size()) {
		Utils::outError("ScreenCodeGenerator::initUseCode",
		                "Maximum % args expected for screen <%>, got %\n\n%",
		                screenNodeVars.size(), node->params, nodeVars.size(), node->getPlace());
	}

	const std::vector<std::string> inner = initCode(screenNode, index);

	res.push_back("_SL_screen_vars_stack.append(screen)");
	res.push_back("_SL_inner_screen_args = SimpleObject()");

	std::vector<std::string> argsWasSet;
	argsWasSet.reserve(nodeVars.size());

	for (size_t i = 0; i < nodeVars.size(); ++i) {
		const auto &[varName, varCode] = nodeVars[i];

		const std::string *namePtr = &varName;
		if (namePtr->empty()) {
			if (i >= screenNodeVars.size()) continue;//error has already been displayed

			namePtr = &screenNodeVars[i].first;
		}
		const std::string &name = *namePtr;

		if (std::find_if(screenNodeVars.cbegin(), screenNodeVars.cend(),
		        [&name] (const auto &p) { return p.first == name; }
		    ) == screenNodeVars.cend()
		) {
			Utils::outError("ScreenCodeGenerator::initUseCode",
			                "No param <%> in screen <%>\n\n%",
			                name, node->params, node->getPlace());
			continue;
		}

		argsWasSet.push_back(name);
		res.push_back("_SL_inner_screen_args." + name + " = " + varCode);
	}

	for (const auto &[varName, varCode] : screenNodeVars) {
		if (std::find(argsWasSet.cbegin(), argsWasSet.cend(), varName) != argsWasSet.cend()) continue;

		if (varCode.empty()) {
			Utils::outError("ScreenCodeGenerator::initUseCode",
			                "No value for param <%> in screen <%>\n\n%",
			                varName, node->params, node->getPlace());
			continue;
		}

		res.push_back("_SL_inner_screen_args." + varName + " = " + varCode);
	}

	res.push_back("screen = _SL_inner_screen_args");
	res.push_back({});
	res.insert(res.end(), inner.begin(), inner.end());
	res.push_back({});
	res.push_back("screen = _SL_screen_vars_stack.pop()");

	return res;
}

static std::vector<std::string> initCode(Node *node, const std::string& index) {
	if (node->isScreenEvent) return {};

	const std::string &command = node->command;
	const std::string &params = node->params;

	bool isMainScreen = command == "screen" && index.empty();
	if (node->isScreenConst && !isMainScreen) return {};

	if (command == "use") return initUseCode(node, index);

	if (command == "for" || command == "while") return initCycleCode(node, index);

	std::vector<std::string> res;
	if (command == "continue" || command == "break") {
		Node *t = node->parent;
		int extraDepth = 0;
		while (t->command != "for" && t->command != "while") {
			t = t->parent;
			++extraDepth;
		}

		res.push_back("# _SL_REAL|" + node->getFileName() + "|" + std::to_string(node->getNumLine()));

		if (command == "break" &&
		    t->childNum + 1 < t->parent->children.size() &&
		    t->parent->children[t->childNum + 1]->command == "else")
		{
			res.push_back("_SL_break" + std::to_string(t->id) + " = True");
		}
		if (extraDepth) {
			for (int i = 0; i < extraDepth - 1; ++i) {
				res.push_back("_SL_stack.pop()");
			}
			res.push_back("_SL_last = _SL_stack.pop()");
		}

		res.push_back(command);
		return res;
	}

	if (node->isScreenProp) {
		res.push_back(String::strip(params));
		res.back() += " # _SL_REAL|" + node->getFileName() + "|" + std::to_string(node->getNumLine());
		return res;
	}

	if (command == "$" || command == "python") {
		size_t startLine = node->getNumLine() + (command == "python");

		const std::vector<std::string> code = String::split(params, "\n");
		for (size_t i = 0; i < code.size(); ++i) {
			res.push_back(code[i] + " # _SL_REAL|" + node->getFileName() + "|" + std::to_string(startLine + i));
		}

		return res;
	}



	if (node->isScreenEnd) {
		res.push_back("(");
		for (Node *child : node->children) {
			if (child->isScreenConst || child->isScreenEvent) continue;

			std::vector<std::string> childRes = initCode(child);

			std::string &propCode = childRes[0];
			propCode.insert(String::firstNotInQuotes(propCode, '#') - 1, ",");
			res.push_back("    " + propCode);
		}
		res.push_back(")");
		return res;
	}


	std::string indent;

	if (command == "screen") {
		if (isMainScreen) {
			res.push_back("_SL_stack = []");
			res.push_back("_SL_screen_vars_stack = []");
			res.push_back("screen = screen_vars['" + params + "']");
			res.push_back({});
		}
	}else
	if (command == "if" || command == "elif" || command == "else") {
		indent = "    ";

		Node *prevNode = node->childNum ? node->parent->children[node->childNum - 1] : nullptr;

		bool isElseAfterCycle = command == "else" && (prevNode->command == "for" || prevNode->command == "while");
		if (isElseAfterCycle) {
			res.push_back("if not _SL_break" + std::to_string(prevNode->id));
		}else {
			res.push_back(command + " " + params);
		}
		res.back() += ": # _SL_REAL|" + node->getFileName() + "|" + std::to_string(node->getNumLine());
		if (isElseAfterCycle) {
			res.push_back(indent + "_SL_last = _SL_stack[-1]");
		}
	}

	res.push_back(indent);
	if (isMainScreen) {
		res.back() += "_SL_" + params + " = ";
	}else {
		res.back() += "_SL_last[" + index + "] = ";
	}
	res.back() += "_SL_last = [";

	size_t bracketStrIndex = res.size() - 1;
	size_t bracketIndex = res.back().size() - 1;

	uint32_t lastPropNum = uint32_t(-1);

	//props before first obj
	for (Node *child : node->children) {
		if (child->isScreenConst || child->isScreenEvent) continue;
		if (!child->isScreenProp) break;

		std::vector<std::string> childRes = initCode(child);

		std::string &propCode = childRes[0];
		propCode.insert(String::firstNotInQuotes(propCode, '#') - 1, ",");
		res.push_back(indent + "    " + propCode);

		lastPropNum = child->screenNum;
	}

	const std::vector<std::pair<int, Node*>> activeChildren = getNumberedActiveChildren(node, nullptr);
	uint32_t maxChildNum = uint32_t(-1);
	for (size_t i = activeChildren.size() - 1; i != size_t(-1); --i) {
		const auto &[index, child] = activeChildren[i];
		if (index < 0) {
			maxChildNum = child->screenNum;
			break;
		}
	}

	if (maxChildNum != uint32_t(-1)) {
		size_t count = maxChildNum - lastPropNum;
		for (size_t i = 0; i < count; ++i) {
			res.push_back(indent + "    None,");
		}
		res.push_back(indent + ']');
	}else {
		res.back() += ']';
	}

	bool empty = false;
	if (lastPropNum == maxChildNum) {
		std::string &bracketLine = res[bracketStrIndex];
		bracketLine[bracketIndex] = '(';
		res.back().back() = ')';

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
		res.push_back(indent);

		for (const auto &[index, child] : activeChildren) {
			if (child->screenNum <= lastPropNum && lastPropNum != uint32_t(-1)) continue;

			initChildCode(child, res, indent, std::to_string(index));

			res.push_back(indent);
		}
	}


	if (isMainScreen || node->withScreenEvent) {
		res.push_back({});
		res.push_back("_SL_check_events()");
	}

	return res;
}
