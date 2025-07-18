#include "parser.h"

#include <algorithm>
#include <fstream>
#include <map>

#include "logger.h"

#include "media/py_utils.h"
#include "media/translation.h"

#include "parser/node.h"
#include "parser/syntax_checker.h"

#include "utils/algo.h"
#include "utils/scope_exit.h"
#include "utils/file_system.h"
#include "utils/string.h"
#include "utils/utils.h"



static void checkPythonSyntax(Node *node) {
	uint32_t line = node->getNumLine() + (node->command != "$");
	PyObject *co = PyUtils::getCompileObject(node->params, node->getFileName(), line);
	if (!co) {
		node->params = "";
	}
}

using Strings = std::initializer_list<std::string>;

static const Strings eventProps = {
	"action", "alternate", "hovered", "unhovered",
	"activate_sound", "hover_sound"
};
static const Strings fakeComps = {
	"if", "elif", "else", "for", "while", "continue", "break", "$", "python"
};
static const Strings comps = {
	"screen", "use", "hbox", "vbox", "null", "image", "imagemap", "hotspot", "text", "textbutton", "button", "key"
};
void Parser::getIsFakeOrIsProp(const std::string &type, bool &isFake, bool &isProp, bool &isEvent) {
	isFake = Algo::in(type, fakeComps);
	isProp = !isFake && !Algo::in(type, comps);
	isEvent = isProp && Algo::in(type, eventProps);
}


static void removeComment(std::string &s) {
	if (s.find('#') == size_t(-1)) return;

	bool q1 = false;
	bool q2 = false;
	for (size_t i = 0; i < s.size(); ++i) {
		const char c = s[i];
		if (c == '\'' && !q2) q1 = !q1;
		if (c == '"'  && !q1) q2 = !q2;

		if (!q1 && !q2) {
			if (c == '#') {
				s.erase(s.begin() + long(i), s.end());
				break;
			}
		}
	}
}


static std::vector<std::string> getFileNames(const std::string &path) {
	if (FileSystem::exists(path)) {
		return FileSystem::getFilesRecursive(path);
	}

	Utils::outError("getFileNames", "Directory <%> not found", path);
	static const std::string mainMenu = "mods/main_menu";
	if (path != mainMenu) {
		return getFileNames(mainMenu);
	}
	return {};
}


static const std::string ops = "(,+-*/=[{";

Parser::Parser(const std::string &dir) {
	this->dir = dir;

	double searchStartTime = Utils::getTimer();
	std::vector<std::string> files = getFileNames(dir);

	std::vector<std::string> commonFiles = getFileNames("mods/common/");
	files.insert(files.begin(), commonFiles.begin(), commonFiles.end());
	std::vector<std::string> engineFiles = getFileNames("../Ren-Engine/rpy/");
	files.insert(files.begin(), engineFiles.begin(), engineFiles.end());
	Logger::logEvent("Searching files (" + std::to_string(files.size()) + ")", Utils::getTimer() - searchStartTime);

	for (const std::string &fileName : files) {
		if (fileName.find("_SL_FILE_") != size_t(-1)) {
			Utils::outError("Parser::Parser", "File <%> must be renamed to not contain <_SL_FILE_>", fileName);
		}
		if (fileName.find("_SL_REAL") != size_t(-1)) {
			Utils::outError("Parser::Parser", "File <%> must be renamed to not contain <_SL_REAL>", fileName);
		}
	}

	double readStartTime = Utils::getTimer();
	int countFiles = 0;
	int countLines = 0;

	for (const std::string &fileName : files) {
		if (!String::endsWith(fileName, ".rpy")) continue;

		bool toPrevLine = false;

		std::string s;
		std::ifstream is(fileName);
		code.push_back("FILENAME " + fileName);
		++countFiles;

		size_t pythonIndent = size_t(-1);

		bool start = true;
		while (!is.eof()) {
			++countLines;
			std::getline(is, s);
			if (start) {
				start = false;
				if (s.size() >= 3 && uint8_t(s[0]) == 239 && uint8_t(s[1]) == 187 && uint8_t(s[2]) == 191) {//utf8 with bom
					s.erase(0, 3);
				}
			}

			String::replaceAll(s, "\t", "    ");

			if (String::startsWith(s, "FILENAME")) {
				Utils::outError("Parser::Parser", "String cannot start with <FILENAME> (file <%>)", fileName);
				s = "";
			}

			size_t n = s.find_last_not_of(' ');
			s.erase(n + 1);

			removeComment(s);
			n = s.find_first_not_of(' ');
			if (n == size_t(-1)) {
				code.push_back({});
				continue;
			}


			if (n <= pythonIndent) {
				if (String::endsWith(s, "python:")) {
					pythonIndent = n;
					toPrevLine = false;
				}else {
					pythonIndent = size_t(-1);
				}
			}

			if (toPrevLine && n <= pythonIndent) {//need move to prev line and not in python-block
				s.erase(0, n);

				for (size_t i = code.size() - 1; i != size_t(-1); --i) {
					if (!code[i].empty()) {
						code[i] += s;
						break;
					}
				}
				code.push_back({});
			}else {
				code.push_back(s);
			}

			toPrevLine = ops.find(s.back()) != size_t(-1);
		}
	}

	const std::string event = "Reading (" + std::to_string(countLines) + " lines from " + std::to_string(countFiles) + " files)";
	Logger::logEvent(event, Utils::getTimer() - readStartTime);
}

Node* Parser::parse() {
	double parsingStartTime = Utils::getTimer();
	Node *res = getMainNode();
	Logger::logEvent("Parsing", Utils::getTimer() - parsingStartTime);

	return res;
}

Node* Parser::getMainNode() {
	Node *res = Node::getNewNode(dir, 0);
	res->parent = nullptr;
	res->command = "main";

	size_t lastSlash = dir.find_last_of('/');
	res->params = dir.substr(lastSlash + 1);


	uint32_t start = 0;
	uint32_t end = uint32_t(code.size());
	const std::string prevHeadWord = "None";
	while (start < end) {
		const std::string &headLine = code[start];
		if (headLine.empty()) {
			++start;
			continue;
		}

		size_t i = headLine.find(' ');
		if (i == size_t(-1)) {
			i = headLine.size();
		}
		if (i && headLine[i - 1] == ':') {
			--i;
		}
		const std::string headWord = headLine.substr(0, i);

		if (headWord == "FILENAME") {
			fileName = headLine.substr(headWord.size() + 1);
			startFile = start;
			++start;
			continue;
		}

		bool t;
		bool ok = SyntaxChecker::check("main", headWord, prevHeadWord, SuperParent::MAIN, t);
		if (!ok) {
			Utils::outError("Parser::getMainNode",
			                "Invalid syntax in string <%>\n\n"
			                "File <%>\n"
			                "Line: %",
			                headLine, fileName, start - startFile);
		}

		uint32_t nextChildStart = start + 1;
		while (nextChildStart < code.size() && (code[nextChildStart].empty() || code[nextChildStart][0] == ' ')) {
			++nextChildStart;
		}

		if (ok) {
			int superParent;
			if (headWord == "label") {
				superParent = SuperParent::LABEL;
			}else
			if (headWord == "init") {
				superParent = SuperParent::INIT;
			}else
			if (headWord == "screen") {
				superParent = SuperParent::SCREEN;
			}else {
				size_t end = headLine.find_last_not_of(':');
				end = headLine.find_last_not_of(' ', end) + 1;
				size_t start = headLine.find_last_of(' ', end - 1) + 1;
				std::string type = headLine.substr(start, end - start);

				if (type == "strings") {
					superParent = SuperParent::TL_STRS;
				}else {
					superParent = SuperParent::LABEL;
				}
			}

			Node *node = getNode(start, nextChildStart, superParent, false);

			if (node->command == "label" || node->command == "screen") {
				if (node->params.find('\'') != size_t(-1) || node->params.find('"') != size_t(-1)) {
					Utils::outError("Parser::getMainNode",
					                "Name of % contains quote: <%>\n%",
					                node->command, node->params, node->getPlace());
					String::replaceAll(node->params, "'", "");
					String::replaceAll(node->params, "\"", "");
				}
			}
			if (node->command == "label") {
				for (const Node *child : res->children) {
					if (node->command == child->command && node->params == child->params) {
						Logger::log("\nRedefinition " + node->command + " <" + node->params + ">:\n" +
						            node->getPlace() + "\n" +
						            "Previous definition:\n" +
						            child->getPlace() + "\n");
						break;
					}
				}
			}

			node->parent = res;
			node->childNum = uint32_t(res->children.size());
			res->children.push_back(node);
		}
		start = nextChildStart;
	}

	code.clear();

	return res;
}


static const Strings compsWithFirstParam = { "screen", "use", "hotspot", "image", "key", "text", "textbutton" };

static void initScreenNode(Node *node) {
	std::vector<Node*> toInsert;

	ScopeExit se([&]() {
		node->children.insert(node->children.begin(), toInsert.begin(), toInsert.end());

		std::map<std::string, Node*> props;

		for (size_t i = 0; i < node->children.size(); ++i) {
			Node *child = node->children[i];
			child->childNum = uint32_t(i);

			if (child->isScreenProp) {
				auto it = props.find(child->command);
				if (it != props.end()) {
					Node *orig = it->second;
					Utils::outError("Parser::initScreenNode",
					                "Redefinition %:\n"
					                "%\n"
					                "Previous definition:\n"
					                "%",
					                child->command, child->getPlace(), orig->getPlace());
				}else {
					props[child->command] = child;
				}
			}
		}
	});


	bool isFakeComp, isScreenProp, isScreenEvent;
	Parser::getIsFakeOrIsProp(node->command, isFakeComp, isScreenProp, isScreenEvent);
	node->isScreenProp = isScreenProp;
	node->isScreenEvent = isScreenEvent;
	if (isFakeComp || isScreenProp) return;


	std::vector<std::string> args = Algo::getArgs(node->params);

	const bool firstIsProp = Algo::in(node->command, compsWithFirstParam);
	if (firstIsProp) {
		if (node->command == "screen" || node->command == "use") {
			size_t i = node->params.find('(');
			if (i == size_t(-1)) return;

			std::string argsStr = node->params.substr(i);
			i = node->params.find_last_not_of(' ', i - 1);
			if (i != size_t(-1)) {
				node->params.erase(i + 1);
			}

			if (!Algo::isLexicalValid(argsStr)) {
				Utils::outError("Parser::initScreenNode",
				                "Screen args are invalid\n\n%",
				                node->getPlace());
				return;
			}

			auto &vars = node->getScreenVars();

			argsStr = Algo::clear(argsStr);
			args = Algo::getArgs(argsStr, ',');
			for (const std::string& arg : args) {
				const std::string_view argSW = std::string_view(arg);

				std::string name, value;
				bool checkName = true;
				bool checkValue = true;
				bool ok = true;

				i = argSW.find('=');
				if (i == size_t(-1)) {
					name = String::strip(argSW);
					if (node->command == "use") {
						value.swap(name);
						checkName = false;
					}else {
						checkValue = false;
					}
				}else {
					name = String::strip(argSW.substr(0, i));
					value = String::strip(argSW.substr(i + 1));
				}

				if (checkName && !Algo::isValidPyName(name)) {
					ok = false;
					Utils::outError("Parser::initScreenNode",
					                "Invalid name <%>\n\n%",
					                name, node->getPlace());
				}
				if (checkValue) {
					PyObject *co = nullptr;
					if (!value.empty()) {
						co = PyUtils::getCompileObject(value, node->getFileName(), node->getNumLine());
					}

					if (!co) {
						ok = false;
						Utils::outError("Parser::initScreenNode",
						                "Invalid default value <%> for argument <%>\n\n%",
						                value, name, node->getPlace());
					}
				}

				if (!name.empty() &&
				    std::find_if(vars.cbegin(), vars.cend(),
				        [&name] (const auto &p) { return p.first == name; }
				    ) != vars.cend()
				) {
					ok = false;
					const char *error;
					if (node->command == "screen") {
						error = "Duplicate argument <%> in screen definition\n\n%";
					}else {// node->command == "use"
						error = "Keyword <%> argument repeated\n\n%";
					}
					Utils::outError("Parser::initScreenNode", error,
					                name, node->getPlace());
				}

				if (ok) {
					vars.push_back({ name, value });
				}
			}

			// messages copypasted from same python2 errors
			for (i = 1; i < vars.size(); ++i) {
				bool prevWithDefault = !vars[i - 1].second.empty();
				bool curWithDefault  = !vars[i].second.empty();
				bool error = prevWithDefault && !curWithDefault;
				if (!error) continue;

				const char *errorStr;
				if (node->command == "screen") {
					//example: screen screen_name(arg1 = None, arg2)
					errorStr = "Non-default argument follows default argument\n\n%";
				}else {// node->command == "use"
					//example: use screen_name(arg2 = None, value1)
					errorStr = "Non-keyword arg after keyword arg\n\n%";
				}
				Utils::outError("Parser::initScreenNode", errorStr,
				                node->getPlace());
				break;
			}

			return;
		}

		if (args.empty()) {
			Utils::outError("Parser::initScreenNode",
			                "Invalid syntax\n"
			                "Object <%> must have main parameter\n\n%",
			                node->command, node->getPlace());
			return;
		}

		Node *firstParam = Node::getNewNode(node->getFileName(), node->getNumLine());
		firstParam->parent = node;
		firstParam->isScreenProp = true;
		firstParam->command = (node->command == "hotspot") ? "crop" : "first_param";
		firstParam->params = args[0];
		toInsert.push_back(firstParam);

		initScreenNode(firstParam);
	}


	for (size_t i = firstIsProp; i < args.size(); i += 2) {
		const std::string &name = args[i];
		bool t;
		if (!SyntaxChecker::check(node->command, name, "", SuperParent::SCREEN, t)) {
			Utils::outError("Parser::initScreenNode",
			                "Invalid syntax in string <% %>\n"
			                "Parameter <%> is not property of <%>\n\n%",
			                node->command, node->params,
			                name, node->command, node->getPlace());
			continue;
		}

		if (i + 1 >= args.size()) {
			Utils::outError("Parser::initScreenNode",
			                "Skip value of parameter <%> in string\n"
			                "<%>\n\n%",
			                name, node->params, node->getPlace());
			continue;
		}
		const std::string &value = args[i + 1];

		Node *param = Node::getNewNode(node->getFileName(), node->getNumLine());
		param->parent = node;
		param->isScreenProp = true;
		param->command = name;
		param->params = value;
		toInsert.push_back(param);

		initScreenNode(param);
	}
}

static const Strings childMayHaveEffect = { "scene", "show", "hide" };

Node* Parser::getNode(uint32_t start, uint32_t end, int superParent, bool isText) {
	Node *res = Node::getNewNode(fileName, start - startFile);

	std::string headLine = code[start];
	headLine.erase(headLine.find_last_not_of(' ') + 1);
	headLine.erase(0, headLine.find_first_not_of(' '));

	if (isText) {
		if (String::startsWith(headLine, "$")) {
			res->command = "$";
			res->params = headLine.substr(1);
			Utils::outError("Parser::getNode",
			                "Do you mean <$ %> instead of <%>?\n\n%",
			                res->params, headLine, res->getPlace());
			return res;
		}

		res->command = "text";
		res->params = headLine;
		return res;
	}

	bool block = start != end - 1;
	if (!block) {
		if (headLine.back() == ':') {
			Utils::outError("Parser::getNode",
			                "Only block declaration ends with a colon\n"
			                "String <%>\n\n%",
			                headLine, res->getPlace());
			headLine.erase(headLine.size() - 1);
		}
		std::string copy = headLine + '\n';
		if (String::firstNotInQuotes(copy, '\n') == size_t(-1)) {
			Utils::outError("Parser::getNode",
			                "Not closed quote in <%>\n\n%",
			                headLine, res->getPlace());
			res->command = "pass";
			return res;
		}
	}else {
		if (headLine.back() != ':') {
			Utils::outError("Parser::getNode",
			                "Block declaration must ends with a colon\n"
			                "String <%>\n\n%",
			                headLine, res->getPlace());
			headLine = "pass";
			block = false;
			end = start + 1;
		}else {
			headLine.pop_back();
		}
	}

	size_t endType = headLine.find_first_of(' ');
	if (endType == size_t(-1)) {
		endType = headLine.size();
	}
	std::string type = res->command = headLine.substr(0, endType);

	if (!block &&
	    (type == "if" || type == "elif" || type == "else" ||
	     type == "for" || type == "while" || type == "python"))
	{
		Utils::outError("Parser::getNode",
		                "Expected an indented block\n"
		                "String <%>\n\n%",
		                headLine, res->getPlace());
		type = res->command = headLine = "pass";
		endType = headLine.size();
	}

	size_t startParams = headLine.find_first_not_of(' ', endType);
	if (startParams == size_t(-1)) {
		startParams = headLine.size();
	}
	size_t endParams = headLine.size();

	if (!block) {
		res->params = headLine.substr(startParams, endParams - startParams);
		if (type == "$") {
			checkPythonSyntax(res);
		}else
		if (type == "continue" || type == "break") {
			size_t indent = code[start].find_first_not_of(' ');

			size_t index = start;
			bool ok = false;
			while (index) {
				--index;

				std::string_view tmp = code[index];
				if (tmp.empty()) continue;

				const size_t tmpIndent = tmp.find_first_not_of(' ');
				if (tmpIndent >= indent) continue;
				if (!tmpIndent) break;

				indent = tmpIndent;

				tmp = tmp.substr(tmpIndent);
				if (String::startsWith(tmp, "for ") || String::startsWith(tmp, "while ")) {
					ok = true;
					break;
				}
			}
			if (!ok) {
				Utils::outError("Parser::getNode",
				                "<%> outside loop\n%",
				                type, res->getPlace());
				type = res->command = "pass";
			}
		}

		if (superParent & SuperParent::SCREEN) {
			initScreenNode(res);
		}
		return res;
	}


	if (type == "init") {
		if (String::endsWith(headLine, " python")) {
			res->command = type = "init python";
		}
		const std::vector<std::string> words = String::split(headLine, " ");

		if (words.size() >= 2 && words[1] != "python") {
			res->priority = float(String::toDouble(words[1]));
		}else {
			res->priority = 0;
		}
	}else {
		res->params = headLine.substr(startParams, endParams - startParams);
		if (type == "translate") {
			std::vector<std::string> words = Algo::getArgs(res->params);
			if (words.size() != 2) {
				Utils::outError("Parser::getNode", "<translate> expected 2 args, got args:\n  <%>", res->params);
				words = { "None", "none_none" };
			}
			const std::string &lang = words[0];
			const std::string &translateType = words[1];

			Translation::addLang(lang);

			if (translateType == "strings") {
				res->command = type = "translate strings";
				res->params = lang;
			}else {
				res->command = type = "label";
				if (translateType.find('_') == size_t(-1)) {
					Utils::outError("Parser::getNode", "Unknown type of translation <%>", translateType);
				}
			}
		}
	}


	if (type == "python" || type == "init python") {
		size_t i = start + 1;

		size_t startIndent;
		do {
			startIndent = code[i++].find_first_not_of(' ');
		}while (startIndent == size_t(-1) && i < end);

		for (i = start + 1; i < end; ++i) {
			const std::string &line = code[i];
			if (line.size() > startIndent) {
				res->params += line.substr(startIndent);
			}
			if (i != end - 1) {
				res->params += '\n';
			}
		}

		checkPythonSyntax(res);
		return res;
	}

	uint32_t childStart = start + 1;
	std::string prevHeadWord = "None";
	while (childStart < end) {
		headLine = code[childStart];

		size_t startLine = headLine.find_first_not_of(' ');
		if (startLine == size_t(-1)) {
			++childStart;
			continue;
		}

		uint32_t nextChildStart = getNextStart(childStart);
		ScopeExit se([&]() {
			childStart = nextChildStart;
		});

		if (type == "menu") {
			headLine.insert(startLine, "menuItem ");
			code[childStart] = headLine;
		}

		size_t i = headLine.find(' ', startLine);
		if (i == size_t(-1)) {
			i = headLine.size();
		}
		if (i && headLine[i - 1] == ':') {
			--i;
		}
		const std::string headWord = headLine.substr(startLine, i - startLine);

		bool isText;
		if (!SyntaxChecker::check(type, headWord, prevHeadWord, superParent, isText)) {
			Utils::outError("Parser::getNode",
			                "Invalid syntax in string <%>\n"
			                "(node: <%>, parentNode: <%>, prevNode: <%>)\n\n%",
			                headLine, headWord, type, prevHeadWord, res->getPlace());
			continue;
		}
		prevHeadWord = headWord;

		Node *node = getNode(childStart, nextChildStart, superParent, isText);
		if (node->command == "has" && node->params != "vbox" && node->params != "hbox") {
			Utils::outError("Parser::getNode",
			                "Invalid syntax in string <%>\n"
			                "After <has> expected <vbox> or <hbox>, got <%>\n\n%",
			                headLine, node->params, node->getPlace());
			continue;
		}

		node->parent = res;

		if (node->command == "with") {
			size_t i = res->children.size() - 1;
			while (i != size_t(-1)) {
				Node *child = res->children[i];
				const std::string &childCommand = child->command;
				std::string &childParams = child->params;

				if (Algo::in(childCommand, childMayHaveEffect)) {
					if (childParams.find(" with ") == size_t(-1)) {
						childParams += " with " + node->params;
					}
					--i;
				}else {
					break;
				}
			}
			++i;
			node->children.insert(node->children.begin(), res->children.begin() + long(i), res->children.end());
			res->children.erase(res->children.begin() + long(i), res->children.end());
		}
		res->children.push_back(node);
	}

	for (uint32_t i = 0; i < res->children.size(); ++i) {
		res->children[i]->childNum = i;
	}

	if (superParent & SuperParent::SCREEN) {
		initScreenNode(res);
	}
	return res;
}

uint32_t Parser::getNextStart(uint32_t start) const {
	if (code[start].empty()) return start + 1;

	const uint32_t countStartIndent = uint32_t(code[start].find_first_not_of(' '));

	uint32_t last = start;
	for (uint32_t i = start + 1; i < code.size(); ++i) {
		const std::string &line = code[i];
		if (line.empty()) continue;

		const size_t countIndent = line.find_first_not_of(' ');
		if (countIndent <= countStartIndent) {
			return last + 1;
		}

		last = i;
	}

	return last + 1;
}
