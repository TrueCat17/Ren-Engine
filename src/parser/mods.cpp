#include "mods.h"

#include <algorithm>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include <Python.h>

#include "media/translation.h"
#include "utils/file_system.h"


struct Mod {
	std::string dir;
	std::map<std::string, std::string> names;
};

static std::vector<Mod> mods;
static PyObject *pyMods = nullptr;


static const std::string modsPath = "mods/";
void Mods::init() {
	mods.clear();

	std::vector<std::string> dirs = FileSystem::getDirectories(modsPath);
	for (const std::string &dir : dirs) {
		if (!FileSystem::exists(dir + "/name")) continue;

		Mod mod;
		mod.dir = dir.substr(modsPath.size());//lost only dirName, without "mods/"

		std::ifstream namesFile(dir + "/name");
		bool start = true;
		while (!namesFile.eof()) {
			std::string tmp;
			std::getline(namesFile, tmp);
			if (tmp.empty()) continue;

			if (start) {
				start = false;
				tmp.insert(0, "None = ");
			}

			size_t separator = tmp.find('=');
			size_t startLang = tmp.find_first_not_of(' ');
			size_t endLang   = tmp.find_last_not_of(' ', separator - 1) + 1;
			size_t startName = tmp.find_first_not_of(' ', separator + 1);
			size_t endName   = tmp.find_last_not_of(' ') + 1;

			std::string lang = tmp.substr(startLang, endLang - startLang);
			std::string name = tmp.substr(startName, endName - startName);
			mod.names[lang] = name;
		}

		if (!mod.names.empty()) {
			mods.push_back(mod);
		}
	}
}

void Mods::clearList() {
	Py_XDECREF(pyMods);
	pyMods = nullptr;
}

static void updateList() {
	const std::string &curLang = Translation::getLang();

	std::vector<std::pair<std::string, std::string>> res;
	res.reserve(mods.size());
	for (const Mod &mod : mods) {
		std::string name;

		auto it = mod.names.find(curLang);
		if (it == mod.names.end()) {
			name = mod.names.at("None");
		}else {
			name = it->second;
		}

		res.push_back({name, mod.dir});
	}
	std::sort(res.begin(), res.end());

	Py_XDECREF(pyMods);
	pyMods = PyTuple_New(long(res.size()));
	for (size_t i = 0; i < res.size(); ++i) {
		PyObject *item = PyTuple_New(2);
		PyTuple_SET_ITEM(item, 0, PyString_FromString(res[i].first.c_str()));
		PyTuple_SET_ITEM(item, 1, PyString_FromString(res[i].second.c_str()));
		PyTuple_SET_ITEM(pyMods, i, item);
	}
}

PyObject* Mods::getList() {
	if (!pyMods) {
		updateList();
	}
	Py_INCREF(pyMods);
	return pyMods;
}
