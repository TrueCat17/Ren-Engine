#include "parser.h"

#include "iostream"
#include <fstream>

#include "utils/syntax_checker.h"
#include "utils/utils.h"


std::map<String, String> Parser::getModNamesAndLabels() {
	std::map<String, String> res;

	std::vector<String> files = Utils::getFileNames(Utils::ROOT + "mods/");

	for (size_t i = 0; i < files.size(); ++i) {
		String fileName = files[i];
		if (!fileName.endsWith(".rpy")) continue;

		String s;
		std::ifstream is(fileName);
		while (!is.eof()) {
			std::getline(is, s);

			if (s.startsWith("$ mods[", false) || s.startsWith("mods[", false)) {
				std::vector<String> tmp = s.split("\"");
				if (tmp.size() != 5) {
					Utils::outMsg(
						"Файл '" + fileName + "',\n"
						"строка <" + s + ">:\n"
						"неверное количество кавычек (ожидалось 4)"
					);
				}else {
					String label = tmp[1];
					String name = tmp[3];

					res[name] = label;
				}
				break;
			}
		}
	}

	return res;
}

Parser::Parser(String dir) {
	std::vector<String> files = Utils::getFileNames(dir);

	std::vector<String> stdFiles = Utils::getFileNames(Utils::ROOT + "mods/inc/");
	files.insert(files.begin(), stdFiles.begin(), stdFiles.end());

	for (int i = 0; i < int(files.size()); ++i) {
		String fileName = files[i];
		if (!fileName.endsWith(".rpy")) continue;

		bool toPrevLine = false;

		String s;
		std::ifstream is(fileName);
		while (!is.eof()) {
			std::getline(is, s);

			s.replaceAll('\t', "    ");

			size_t n = s.find_last_not_of(' ');
			s.erase(n + 1);

			n = s.find_first_not_of(' ');
			if (n != size_t(-1) && s[n] != '#') {
				if (toPrevLine) {
					s.erase(0, n);
					code[code.size() - 1] += s;
				}else {
					code.push_back(s);
				}
			}else {
				code.push_back(String());
			}

			if (s) {
				char last = s[s.size() - 1];

				static String ops = "(,+-*/=[{";
				toPrevLine = (ops.find(last) != size_t(-1));
			}
		}
	}
}

Node* Parser::parse() {
	return getMainNode();
}

Node* Parser::getMainNode() {
	Node *res = new Node();
	res->command = "main";

	size_t start = 0;
	size_t end = code.size();
	String prevHeadWord = "None";
	while (start < end) {
		String headLine = code[start];

		size_t i = headLine.find(' ');
		if (i == size_t(-1)) {
			i = headLine.size();
		}
		String headWord = headLine.substr(0, i);
		if (headWord.endsWith(":")) {
			headWord.erase(i - 1);
		}

		bool t;
		bool ok = SyntaxChecker::check("main", headWord, prevHeadWord, SuperParent::MAIN, t);
		if (!ok) {
			Utils::outMsg("Parser::getMainNode", "Неверный синтаксис в строке <" + headLine + ">");
		}
		prevHeadWord = headWord;

		size_t nextChildStart = start + 1;
		while (nextChildStart < code.size() && (!code[nextChildStart] || code[nextChildStart][0] == ' ')) {
			++nextChildStart;
		}

		if (ok) {
			const int superParent = headWord == "label" ? SuperParent::LABEL : headWord == "init" ? SuperParent::INIT : SuperParent::SCREEN;
			Node *node = getNode(start, nextChildStart, superParent, false);
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
			node->props[childNode->command] = childNode->params;
		}
	}
	if (initOnlyChildren) return;

	bool firstIsProp = Utils::in(node->command, compsWithFirstParam);
	std::vector<String> args = Utils::getArgs(node->params);
	if (firstIsProp) {
		node->firstParam = args[0];
	}
	for (size_t i = firstIsProp; i < args.size(); i += 2) {
		String name = args[i];
		bool t;
		if (!SyntaxChecker::check(node->command, name, "", SuperParent::SCREEN, t)) {
			String str = node->command + ' ' + node->params;
			Utils::outMsg("Parser::initScreenNode", "Неверный синтаксис в строке <" + str + ">\n"
						  "Параметр <" + name + "> не является свойством объекта типа <" + node->command + ">");
			continue;
		}

		if (i + 1 >= args.size()) {
			Utils::outMsg("Parser::initScreenNode",
						  "У параметра <" + name + "> пропущено значение в строке\n" +
						  "<" + node->params + ">");
			break;
		}
		String value = args[i + 1];
		node->props[name] = value;
	}
}

Node* Parser::getNode(size_t start, size_t end, int superParent, bool isText) {
	Node *res = new Node();

	String headLine = code[start];

	if (isText) {
		res->command = "text";
		res->params = headLine;
		return res;
	}

	if (start == end - 1) {
		size_t startLine = headLine.find_first_not_of(' ');

		size_t i = headLine.find(' ', startLine);
		if (i == size_t(-1)) {
			i = headLine.size();
		}
		String headWord = headLine.substr(startLine, i - startLine);

		res->command = headWord;
		if (i + 1 < headLine.size()) {
			res->params = headLine.substr(i + 1);
		}else {
			res->params = "";
		}

		if (res->command == "$") {
			res->command = "python";
		}

		if (superParent & SuperParent::SCREEN) {
			initScreenNode(res);
		}
		return res;
	}

	String type;
	int wasNotSpace = -1;
	for (size_t i = 0; i < headLine.size(); ++i) {
		char c = headLine[i];
		if (wasNotSpace == -1 && c != ' ') {
			wasNotSpace = i;
		}

		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && wasNotSpace != -1) {
			res->command = type = headLine.substr(wasNotSpace, i - wasNotSpace);
			break;
		}
	}

	size_t i = headLine.size() - 1;
	while (i != size_t(-1) && headLine[i] == ' ') {
		--i;
	}
	headLine.erase(i + 1);

	i = headLine.find(type);
	size_t startCommandName = i + type.size() + 1;
	size_t endCommandName = headLine.size() - 1;
	if (type == "label" || type == "screen") {
		res->name = headLine.substr(startCommandName, endCommandName - startCommandName);
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
		res->params = headLine.substr(startCommandName, endCommandName - startCommandName);
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
			i = headLine.find(' ', startLine);
			if (i == size_t(-1)) {
				i = headLine.size();
			}
			headWord = headLine.substr(startLine, i - startLine);
			if (headWord.endsWith(":")){
				headWord.erase(headWord.size() - 1);
			}
		}

		bool isText;
		if (!SyntaxChecker::check(type, headWord, prevHeadWord, superParent, isText)) {
			Utils::outMsg("Parser::getNode", "Неверный синтаксис в строке <" + headLine + ">\n"
						  "(node: " + headWord + ", parentNode: " + type + ", prevNode: " + prevHeadWord + ")");
			childStart = getNextStart(childStart);
			continue;
		}
		prevHeadWord = headWord;

		size_t nextChildStart = getNextStart(childStart);
		Node *node = getNode(childStart, nextChildStart, superParent, isText);
		if (node->command == "elif" || node->command == "else") {
			node->prevNode = res->children[res->children.size() - 1];
		}else
		if (type == "menuItem") {
			if (headWord == "$") {
				headWord = "python";
			}
			node->command = headWord;
			node->params = headLine.substr(i + (headLine[i] == ' '));
		}

		res->children.push_back(node);

		childStart = nextChildStart;
	}

	if (type == "menu" && !res->children.size()) {
		Utils::outMsg("Parse::getNode", "Меню без пунктов выбора недопустимо");
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
