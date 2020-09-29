#include "parser.h"

#include <set>
#include <map>
#include <fstream>

#include "logger.h"

#include "media/py_utils.h"

#include "parser/node.h"
#include "parser/syntax_checker.h"
#include "parser/screen_node_utils.h"

#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"



static void checkPythonSyntax(Node *node) {
	PyCodeObject *co = PyUtils::getCompileObject(node->params.c_str(), node->getFileName(), node->getNumLine());
	if (!co) {
		PyUtils::errorProcessing(node->params);
		node->params = "";
	}
}


static const std::set<std::string> eventProps({
	"action", "alternate", "hovered", "unhovered",
	"activate_sound", "hover_sound"
});
bool Parser::isEvent(const std::string &type) {
	return eventProps.count(type);
}

static const std::set<std::string> fakeComps({
	"if", "elif", "else", "for", "while", "continue", "break", "$", "python"
});
static const std::set<std::string> comps({
	"screen", "use", "hbox", "vbox", "null", "image", "imagemap", "hotspot", "text", "textbutton", "button", "key"
});
void Parser::getIsFakeOrIsProp(const std::string &type, bool &isFake, bool &isProp, bool &isEvent) {
	isFake = fakeComps.count(type);
	isProp = !isFake && !comps.count(type);
	isEvent = isProp && Parser::isEvent(type);
}


static const std::string modsPath = "mods/";
PyObject* Parser::getMods() {
	std::vector<std::pair<std::string, std::string>> res;

	std::vector<std::string> dirs = FileSystem::getDirectories(modsPath);
	for (std::string dir : dirs) {
		if (!FileSystem::exists(dir + "/name")) continue;

		std::ifstream nameFile(dir + "/name");
		std::string modName;
		std::getline(nameFile, modName);

		dir.erase(0, modsPath.size());//lost only dirName, without "mods/"

		res.push_back({modName, dir});
	}
	std::sort(res.begin(), res.end());

	PyObject *pyRes = PyTuple_New(long(res.size()));
	for (size_t i = 0; i < res.size(); ++i) {
		PyObject *item = PyTuple_New(2);
		PyTuple_SET_ITEM(item, 0, PyString_FromString(res[i].first.c_str()));
		PyTuple_SET_ITEM(item, 1, PyString_FromString(res[i].second.c_str()));
		PyTuple_SET_ITEM(pyRes, i, item);
	}
	return pyRes;
}

Parser::Parser(const std::string &dir) {
	this->dir = dir;

	long searchStartTime = Utils::getTimer();
	std::vector<std::string> files = Utils::getFileNames(dir);

	std::vector<std::string> commonFiles = Utils::getFileNames("mods/common/");
	files.insert(files.begin(), commonFiles.begin(), commonFiles.end());
	std::vector<std::string> engineFiles = Utils::getFileNames("mods/engine/");
	files.insert(files.begin(), engineFiles.begin(), engineFiles.end());
	Logger::logEvent("Searching files (" + std::to_string(files.size()) + ")", Utils::getTimer() - searchStartTime);

	for (const std::string &fileName : files) {
		if (fileName.find("_SL_FILE_") != size_t(-1)) {
			Utils::outMsg("Parser::Parser", "File <" + fileName + "> must be renamed to not contain <_SL_FILE_>");
		}
	}

	long readStartTime = Utils::getTimer();
	int countFiles = 0;
	int countLines = 0;

	for (const std::string &fileName : files) {
		if (!String::endsWith(fileName, ".rpy")) continue;

		bool toPrevLine = false;

		std::string s;
		std::ifstream is(fileName);
		code.push_back("FILENAME " + fileName);
		++countFiles;

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
				Utils::outMsg("Parser::Parser", "String cannot begin with FILENAME (file <" + fileName + ">");
				s = "";
			}

			size_t n = s.find_last_not_of(' ');
			s.erase(n + 1);

			n = s.find_first_not_of(' ');
			if (n <= pythonIndent) {
				pythonIndent = String::endsWith(s, "python:") ? n : size_t(-1);
			}

			if (n != size_t(-1) && s[n] != '#') {
				if (toPrevLine && n <= pythonIndent) {
					s.erase(0, n);

					for (size_t j = code.size() - 1; j != size_t(-1) ; --j) {
						if (!code[j].empty()) {
							code[j] += s;
							break;
						}
					}
				}else {
					bool q1 = false;
					bool q2 = false;
					for (size_t i = n; i < s.size(); ++i) {
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
					code.push_back(s);
				}
			}else {
				code.push_back({});
			}

			if (!s.empty()) {
				const char last = s.back();

				static const std::string opsStr = "(,+-*/=[{";
				static const std::set<char> ops(opsStr.begin(), opsStr.end());
				toPrevLine = ops.count(last);
			}
		}
	}

	const std::string event = "Reading (" + std::to_string(countLines) + " lines from " + std::to_string(countFiles) + " files)";
	Logger::logEvent(event, Utils::getTimer() - readStartTime);
}

Node* Parser::parse() {
	long parsingStartTime = Utils::getTimer();
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


	size_t start = 0;
	size_t end = code.size();
	std::string prevHeadWord = "None";
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
			Utils::outMsg("Parser::getMainNode",
			              "Invalid syntax in string <" + headLine + ">\n\n" +
			              "File <" + fileName + ">\n"
			              "Line: " + std::to_string(start - startFile));
		}
		prevHeadWord = headWord;

		size_t nextChildStart = start + 1;
		while (nextChildStart < code.size() && (code[nextChildStart].empty() || code[nextChildStart][0] == ' ')) {
			++nextChildStart;
		}

		if (ok) {
			const int superParent = headWord == "label" ? SuperParent::LABEL : headWord == "init" ? SuperParent::INIT : SuperParent::SCREEN;
			Node *node = getNode(start, nextChildStart, superParent, false);

			if (node->command == "label" || node->command == "screen") {
				for (Node *child : res->children) {
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
			node->childNum = res->children.size();
			res->children.push_back(node);
		}
		start = nextChildStart;
	}

	code.clear();

	return res;
}

static void initScreenNode(Node *node) {
	ScopeExit se([&]() {
		std::map<std::string, Node*> props;

		for (size_t i = 0; i < node->children.size(); ++i) {
			Node *child = node->children[i];
			child->childNum = i;

			if (child->isScreenProp) {
				auto it = props.find(child->command);
				if (it != props.end()) {
					Node *orig = it->second;
					Utils::outMsg("Parser::initScreenNode",
					              "Redefinition " + child->command + ":\n" +
					              child->getPlace() + "\n" +
					              "Previous definition:\n" +
					              orig->getPlace());
				}else {
					props[child->command] = child;
				}
			}
		}
	});


	bool isFakeComp;
	Parser::getIsFakeOrIsProp(node->command, isFakeComp, node->isScreenProp, node->isScreenEvent);
	if (isFakeComp || node->isScreenProp) {
		return;
	}


	static const std::set<std::string> compsWithFirstParam({"screen", "use", "hotspot", "image", "key", "text", "textbutton"});
	const std::vector<std::string> args = Algo::getArgs(node->params);

	std::vector<Node*> toInsert;

	const bool firstIsProp = compsWithFirstParam.count(node->command);
	if (firstIsProp && node->command != "screen" && node->command != "use") {
		if (args.empty()) {
			Utils::outMsg("Parser::initScreenNode",
			              "Invalid syntax\n"
			              "Object <" + node->command + "> must have main parameter\n\n" +
						  node->getPlace());
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
			const std::string str = node->command + ' ' + node->params;
			Utils::outMsg("Parser::initScreenNode",
			              "Invalid syntax in string <" + str + ">\n"
			              "Parameter <" + name + "> is not property of <" + node->command + ">\n\n" +
						  node->getPlace());
			continue;
		}

		if (i + 1 >= args.size()) {
			Utils::outMsg("Parser::initScreenNode",
			              "Skip value of parameter <" + name + "> in string\n" +
						  "<" + node->params + ">\n\n" +
						  node->getPlace());
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

	node->children.insert(node->children.begin(), toInsert.begin(), toInsert.end());
}

Node* Parser::getNode(size_t start, size_t end, int superParent, bool isText) {
	Node *res = Node::getNewNode(fileName, start - startFile);

	std::string headLine = code[start];
	headLine.erase(headLine.find_last_not_of(' ') + 1);
	headLine.erase(0, headLine.find_first_not_of(' '));

	if (isText) {
		if (headLine.size() > 1 && headLine[0] == '$') {
			Utils::outMsg("SyntaxChecker::check",
			              "Do you mean <$ " + headLine.substr(1) + "> instead <" + headLine + ">?");
			headLine = "'''" + headLine + "'''";
		}

		res->command = "text";
		res->params = headLine;
		return res;
	}

	bool block = start != end - 1;
	if (!block) {
		if (headLine.back() == ':') {
			Utils::outMsg("Parser::getNode",
			              "Only block declaration ends with a colon\n"
			              "String <" + headLine + ">\n\n" +
			              res->getPlace());
			headLine.erase(headLine.size() - 1);
		}
	}else {
		if (headLine.back() != ':') {
			Utils::outMsg("Parser::getNode",
			              "Block declaration must ends with a colon\n"
			              "String <" + headLine + ">\n\n" +
			              res->getPlace());
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
		Utils::outMsg("Parser::getNode",
		              "Expected an indented block\n"
		              "String <" + headLine + ">\n\n" +
		              res->getPlace());
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

				const std::string &tmp = code[index];
				if (tmp.empty()) continue;

				const size_t tmpIndent = tmp.find_first_not_of(' ');
				if (tmpIndent >= indent) continue;
				if (!tmpIndent) break;

				indent = tmpIndent;

				if (String::startsWith(tmp, "for ", true) || String::startsWith(tmp, "while ", true)) {
					ok = true;
					break;
				}
			}
			if (!ok) {
				Utils::outMsg("Parser::getNode",
				              "<" + type + "> outside loop\n" +
				              res->getPlace());
				type = res->command = "pass";
			}
		}

		if (superParent & SuperParent::SCREEN) {
			initScreenNode(res);
		}
		return res;
	}


	if (String::startsWith(headLine, "init")) {
		if (String::endsWith(headLine, " python")) {
			res->command = type = "init python";
		}
		const std::vector<std::string> words = String::split(headLine, " ");

		if (words.size() >= 2 && words[1] != "python") {
			res->priority = String::toDouble(words[1]);
		}else {
			res->priority = 0;
		}
	}else {
		res->params = headLine.substr(startParams, endParams - startParams);
	}

	if (type == "python" || type == "init python") {
		size_t i = start + 1;

		size_t startIndent;
		do {
			startIndent = code[i++].find_first_not_of(' ');
		}while (startIndent == size_t(-1));

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

	size_t childStart = start + 1;
	std::string prevHeadWord = "None";
	while (childStart < end) {
		headLine = code[childStart];

		size_t startLine = headLine.find_first_not_of(' ');
		if (startLine == size_t(-1)) {
			++childStart;
			continue;
		}

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
			Utils::outMsg("Parser::getNode",
			              "Invalid syntax in string <" + headLine + ">\n"
						  "(node: <" + headWord + ">, parentNode: <" + type + ">, prevNode: <" + prevHeadWord + ">)\n\n" +
						  res->getPlace());
			childStart = getNextStart(childStart);
			continue;
		}
		prevHeadWord = headWord;

		size_t nextChildStart = getNextStart(childStart);
		Node *node = getNode(childStart, nextChildStart, superParent, isText);
		node->parent = res;

		if (node->command == "with") {
			size_t i = res->children.size() - 1;
			while (i != size_t(-1)) {
				const static std::set<std::string> canChildWith = {"scene", "show", "hide"};

				Node *child = res->children[i];
				const std::string &childCommand = child->command;
				std::string &childParams = child->params;

				if (canChildWith.count(childCommand) && childParams.find(" with ") == size_t(-1)) {
					childParams += " with " + node->params;
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

		childStart = nextChildStart;
	}

	for (size_t i = 0; i < res->children.size(); ++i) {
		res->children[i]->childNum = i;
	}

	if (superParent & SuperParent::SCREEN) {
		initScreenNode(res);
	}
	return res;
}

size_t Parser::getNextStart(size_t start) const {
	if (code[start].empty()) return start + 1;

	const size_t countStartIndent = code[start].find_first_not_of(' ');

	size_t last = start;
	for (size_t i = start + 1; i < code.size(); ++i) {
		const std::string &line = code[i];
		if (!line.empty()) {
			const size_t countIndent = line.find_first_not_of(' ');
			if (countIndent <= countStartIndent) {
				return last + 1;
			}

			last = i;
		}
	}

	return last + 1;
}
