#include "parser.h"

#include <fstream>

#include "logger.h"

#include "parser/syntax_checker.h"
#include "parser/node.h"

#include "utils/utils.h"


const std::string Parser::getMods() {
	std::string res;

	std::vector<String> files = Utils::getFileNames(Utils::ROOT + "mods/");

	for (size_t i = 0; i < files.size(); ++i) {
		String fileName = files[i];
		if (!fileName.endsWith(".rpy")) continue;

		String s;
		std::ifstream is(fileName);
		while (!is.eof()) {
			std::getline(is, s);

			if (s.startsWith("$ mods[", false) || s.startsWith("mods[", false)) {
				std::vector<String> tmp1 = s.split("'");
				std::vector<String> tmp2 = s.split("\"");
				std::vector<String> &tmp = tmp1.size() == 5 ? tmp1 : tmp2;

				if (tmp.size() != 5) {
					Utils::outMsg(
						"Файл <" + fileName + ">,\n"
						"Строка <" + s + ">:\n"
						"Неверное количество двойных кавычек (ожидалось 4)"
					);
				}else {
					String label = tmp[1];
					String name = tmp[3];

					if (res.size()) {
						res += ", ";
					}
					res += '"' + name + "\":\"" + label + '"';
				}
				break;
			}
		}
	}

	return "{" + res + "}";
}

Parser::Parser(const String &dir) {
	this->dir = dir;

	int searchStartTime = Utils::getTimer();
	std::vector<String> files = Utils::getFileNames(dir + "/");

	std::vector<String> commonFiles = Utils::getFileNames(Utils::ROOT + "mods/common/");
	files.insert(files.begin(), commonFiles.begin(), commonFiles.end());
	std::vector<String> engineFiles = Utils::getFileNames(Utils::ROOT + "mods/engine/");
	files.insert(files.begin(), engineFiles.begin(), engineFiles.end());
	Logger::logEvent("Searching files (" + String(files.size()) + ")", Utils::getTimer() - searchStartTime);

	int readStartTime = Utils::getTimer();
	int countFiles = 0;
	int countLines = 0;

	for (size_t i = 0; i < files.size(); ++i) {
		String fileName = files[i];
		if (!fileName.endsWith(".rpy")) continue;

		bool toPrevLine = false;

		String s;
		std::ifstream is(fileName);
		code.push_back("FILENAME " + fileName);
		++countFiles;

		while (!is.eof()) {
			++countLines;
			std::getline(is, s);

			s.replaceAll('\t', "    ");

			if (s.startsWith("FILENAME")) {
				Utils::outMsg("Parser::Parser", "Строка не может начинаться с FILENAME (файл <" + fileName + ">");
				s = "";
			}

			size_t n = s.find_last_not_of(' ');
			s.erase(n + 1);

			n = s.find_first_not_of(' ');
			if (n <= pythonIndent) {
				pythonIndent = s.endsWith("python:") ? n : size_t(-1);
			}

			if (n != size_t(-1) && s[n] != '#') {
				if (toPrevLine && n <= pythonIndent) {
					s.erase(0, n);

					for (size_t j = code.size() - 1; j != size_t(-1) ; --j) {
						if (code[j]) {
							code[j] += s;
							break;
						}
					}
				}else {
					code.push_back(s);
				}
			}else {
				code.push_back(String());
			}

			if (s) {
				char last = s.back();

				static const String ops = "(,+-*/=[{";
				toPrevLine = ops.find(last) != size_t(-1);
			}
		}
	}

	String event = "Reading (" + String(countLines) + " lines from " + String(countFiles) + " files)";
	Logger::logEvent(event, Utils::getTimer() - readStartTime);
}

Node* Parser::parse() {
	int parsingStartTime = Utils::getTimer();
	Node *res = getMainNode();
	Logger::logEvent("Parsing", Utils::getTimer() - parsingStartTime);

	return res;
}

Node* Parser::getMainNode() {
	Node *res = new Node(dir, 0);
	res->command = "main";

	size_t lastSlash = dir.find_last_of('/');
	res->name = dir.substr(lastSlash + 1);


	size_t start = 0;
	size_t end = code.size();
	String prevHeadWord = "None";
	while (start < end) {
		String headLine = code[start];
		if (!headLine) {
			++start;
			continue;
		}

		size_t i = headLine.find(' ');
		if (i == size_t(-1)) {
			i = headLine.size();
		}
		String headWord = headLine.substr(0, i);
		if (headWord.endsWith(":")) {
			headWord.erase(i - 1);
		}

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
						  "Неверный синтаксис в строке <" + headLine + ">\n\n" +
						  "Файл <" + fileName + ">\n"
						  "Номер строки: " + String(start - startFile));
		}
		prevHeadWord = headWord;

		size_t nextChildStart = start + 1;
		while (nextChildStart < code.size() && (!code[nextChildStart] || code[nextChildStart][0] == ' ')) {
			++nextChildStart;
		}

		if (ok) {
			const int superParent = headWord == "label" ? SuperParent::LABEL : headWord == "init" ? SuperParent::INIT : SuperParent::SCREEN;
			Node *node = getNode(start, nextChildStart, superParent, false);
			node->childNum = res->children.size();
			res->children.push_back(node);
		}
		start = nextChildStart;
	}

	return res;
}

void Parser::initScreenNode(Node *node) {
	static const String compsWithFirstParamStr = "key, text, textbutton, imagebutton, image, hotspot";
	static const std::vector<String> compsWithFirstParam = String(compsWithFirstParamStr).split(", ");
	static const std::vector<String> compsMbWithParams = String(compsWithFirstParamStr + ", button, null, window, vbox, hbox").split(", ");

	bool initOnlyChildren = !Utils::in(node->command, compsMbWithParams);
	for (Node *childNode : node->children) {
		if (childNode && childNode->children.empty()) {
			node->initProp(childNode->command, childNode->params, childNode->getNumLine());
		}
	}
	if (initOnlyChildren) return;

	bool firstIsProp = Utils::in(node->command, compsWithFirstParam);
	std::vector<String> args = Utils::getArgs(node->params);
	if (firstIsProp) {
		node->setFirstParam(args[0]);
	}
	for (size_t i = firstIsProp; i < args.size(); i += 2) {
		String name = args[i];
		bool t;
		if (!SyntaxChecker::check(node->command, name, "", SuperParent::SCREEN, t)) {
			String str = node->command + ' ' + node->params;
			Utils::outMsg("Parser::initScreenNode",
						  "Неверный синтаксис в строке <" + str + ">\n"
						  "Параметр <" + name + "> не является свойством объекта типа <" + node->command + ">\n\n" +
						  node->getPlace());
			continue;
		}

		if (i + 1 >= args.size()) {
			Utils::outMsg("Parser::initScreenNode",
						  "У параметра <" + name + "> пропущено значение в строке\n" +
						  "<" + node->params + ">\n\n" +
						  node->getPlace());
			break;
		}
		String value = args[i + 1];
		node->initProp(name, value, node->getNumLine());
	}
}

Node* Parser::getNode(size_t start, size_t end, int superParent, bool isText) {
	Node *res = new Node(fileName, start - startFile);

	String headLine = code[start];
	headLine.erase(0, headLine.find_first_not_of(' '));
	headLine.erase(headLine.find_last_not_of(' ') + 1);


	if (isText) {
		res->command = "text";
		res->params = headLine;
		return res;
	}

	bool block = start != end - 1;

	size_t endType = headLine.find_first_of(' ');
	if (endType == size_t(-1)) {
		endType = headLine.size() - block;
	}
	String type = res->command = headLine.substr(0, endType);

	size_t startParams = endType + 1;
	size_t endParams = headLine.size() - 1;
	if (startParams > endParams) {
		startParams = endParams + 1;
	}

	if (!block) {
		res->params = headLine.substr(startParams, endParams - startParams + 1);
		if (superParent & SuperParent::SCREEN) {
			initScreenNode(res);
		}
		return res;
	}


	if (headLine.back() != ':') {
		Utils::outMsg("Parser::getNode",
					  "Объявление блока должно заканчиваться двоеточием\n"
					  "Строка <" + headLine + ">\n\n" +
					  res->getPlace());
	}

	if (type == "label" || type == "screen") {
		res->name = headLine.substr(startParams, endParams - startParams);
	}else
	if (headLine.startsWith("init")) {
		if (headLine.endsWith(" python") || headLine.endsWith(" python:")) {
			res->command = type = "init python";
		}
		std::vector<String> words = headLine.split(' ');
		if (words.size() >= 2 && words[1] != "python") {
			res->priority = words[1].toDouble();
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
			String line = code[i];
			if (line.size() > startIndent) {
				line = line.substr(startIndent);
			}else {
				line = "";
			}
			res->params += line + '\n';
		}

		return res;
	}

	size_t childStart = start + 1;
	String prevHeadWord = "None";
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

		String headWord;
		if (headLine[startLine] != '\'' && headLine[startLine] != '"') {
			size_t i = headLine.find(' ', startLine);
			if (i == size_t(-1)) {
				i = headLine.size();
			}
			headWord = headLine.substr(startLine, i - startLine);
		}
		if (headWord.endsWith(":")){
			headWord.erase(headWord.size() - 1);
		}

		bool isText;
		if (!SyntaxChecker::check(type, headWord, prevHeadWord, superParent, isText)) {
			Utils::outMsg("Parser::getNode",
						  "Неверный синтаксис в строке <" + headLine + ">\n"
						  "(node: <" + headWord + ">, parentNode: <" + type + ">, prevNode: <" + prevHeadWord + ">)\n\n" +
						  res->getPlace());
			childStart = getNextStart(childStart);
			continue;
		}
		prevHeadWord = headWord;

		size_t nextChildStart = getNextStart(childStart);
		Node *node = getNode(childStart, nextChildStart, superParent, isText);
		if (node->command == "elif" || node->command == "else") {
			node->prevNode = res->children[res->children.size() - 1];
		}

		if (node->command == "with") {
			size_t i = res->children.size() - 1;
			while (i != size_t(-1) &&
			 (res->children[i]->command == "scene" ||
			  res->children[i]->command == "show" ||
			  res->children[i]->command == "hide"))
			{
				res->children[i]->params += " with " + node->params;
				--i;
			}
			++i;
			node->children.insert(node->children.begin(), res->children.begin() + i, res->children.end());
			res->children.erase(res->children.begin() + i, res->children.end());
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
	String line = code[start];
	if (!line) return start + 1;

	size_t countStartIndent = line.find_first_not_of(' ');

	size_t i = start + 1;
	size_t last = start;
	for (; i < code.size(); ++i) {
		line = code[i];
		if (!line) continue;

		size_t countIndent = line.find_first_not_of(' ');
		if (countIndent <= countStartIndent) {
			return (i - last != 1) ? last + 1 : i;
		}

		last = i;
	}

	return (i - last != 1) ? last + 1 : i;
}
