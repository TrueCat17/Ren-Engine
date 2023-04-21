#include "translation.h"

#include <algorithm>
#include <map>
#include <set>

#include "media/py_utils.h"
#include "parser/mods.h"
#include "utils/utils.h"


static bool enabled;
static std::string curLang;
static std::set<std::string> langs;
static std::map<std::string, std::string> map;

static void update() {
	Mods::clearList();

	map.clear();
	for (const Node *node : GV::mainExecNode->children) {
		if (node->command == "translate strings" && node->params == curLang) {
			std::string prev = "new";
			std::string prevValue;
			for (const Node *child : node->children) {
				if (child->command == "old") {
					prevValue = PyUtils::exec(child->getFileName(), child->getNumLine(), child->params, true);
				}else {
					map[prevValue] = PyUtils::exec(child->getFileName(), child->getNumLine(), child->params, true);
					prevValue.clear();
				}
			}
			if (!prevValue.empty()) {
				Utils::outMsg("Translation::update",
				              "Expected <new> before end of translation block\n" + node->getPlace());
			}
		}
	}
}

void Translation::init() {
	enabled = false;
	curLang = "None";
	langs.clear();
	map.clear();
}
void Translation::addLang(const std::string &lang) {
	langs.insert(lang);
}

const std::string& Translation::getLang() {
	return curLang;
}
void Translation::setLang(const std::string &lang) {
	if (lang == curLang) return;

	curLang = lang;
	if (enabled) {
		update();
	}
}
void Translation::enable() {
	enabled = true;
	update();
}



const std::string& Translation::get(const std::string &str) {
	auto it = map.find(str);
	if (it == map.end()) {
		return str;
	}
	return it->second;
}

PyObject* Translation::getKnownLanguages() {
	std::string defaultLang = PyUtils::exec("CPP_EMBED: translation.cpp", __LINE__, "config.default_language", true);

	std::vector<std::string> vec(langs.begin(), langs.end());
	if (defaultLang != "empty" && langs.find(defaultLang) == langs.end()) {
		vec.push_back(defaultLang);
	}
	std::sort(vec.begin(), vec.end());

	PyObject *res = PyTuple_New(long(vec.size()));
	for (size_t i = 0; i < vec.size(); ++i) {
		PyTuple_SET_ITEM(res, i, PyUnicode_FromString(vec[i].c_str()));
	}
	return res;
}
