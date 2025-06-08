#include "mods.h"

#include <algorithm>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include "media/translation.h"
#include "media/py_utils/convert_to_py.h"

#include "utils/file_system.h"
#include "utils/string.h"
#include "utils/utils.h"


struct Mod {
	std::string dir;
	std::map<std::string, std::string> names;
};

static std::vector<Mod> mods;
static PyObject *pyMods = nullptr;


static const std::string modsPath = "mods/";
void Mods::init() {
	std::vector<std::string> dirs = FileSystem::getDirectories(modsPath);
	for (const std::string &dir : dirs) {
		std::string file = dir + "/name";
		if (!FileSystem::exists(file)) continue;

		Mod mod;
		mod.dir = dir.substr(modsPath.size());//lost only dirName, without "mods/"

		std::ifstream nameFile(file);
		while (!nameFile.eof()) {
			std::string line;
			std::getline(nameFile, line);
			if (line.empty()) continue;

			if (mod.names.find("None") == mod.names.end()) {
				mod.names["None"] = String::strip(line);
				continue;
			}

			size_t separator = line.find('=');
			if (separator == size_t(-1)) {
				Utils::outError("Mods::init", "Line <%> in file <%> is incorrect", line, file);
				continue;
			}

			std::string lang = String::strip(line.substr(0, separator));
			std::string name = String::strip(line.substr(separator + 1));
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
		PyTuple_SET_ITEM(item, 0, convertToPy(res[i].first));
		PyTuple_SET_ITEM(item, 1, convertToPy(res[i].second));
		PyTuple_SET_ITEM(pyMods, Py_ssize_t(i), item);
	}
}

PyObject* Mods::getList() {
	if (!pyMods) {
		updateList();
	}
	Py_INCREF(pyMods);
	return pyMods;
}
