#include "config.h"

#include <fstream>

#include "utils/utils.h"


std::vector<Param> Config::params;
bool Config::initing = false;
bool Config::changed = false;

void Config::init() {
	initing = true;
	setDefault();
	load();
	initing = false;
}

String Config::get(const String &name) {
	for (size_t i = 0; i < params.size(); ++i) {
		if (params[i].name == name) {
			return params[i].value;
		}
	}

	Utils::outMsg("Config::get", "Попытка получить значение несуществующего параметра <" + name + ">");
	return "";
}

void Config::set(const String &name, const String &value, const String &comment) {
	bool assigned = false;
	for (size_t i = 0; i < params.size(); ++i) {
		Param &param = params[i];
		if (param.name == name) {
			if (param.value == value && param.comment == comment) {
				return;
			}

			param.value = value;
			if (comment) {
				param.comment = comment;
			}

			assigned = true;
			break;
		}
	}

	if (!assigned) {
		if (initing) {
			params.push_back({name, value, comment});
		}else {
			Utils::outMsg("Config::set", "Попытка установить значение несуществующего параметра <" + name + ">");
		}
	}

	if (!initing) {
		changed = true;
	}
}

void Config::setDefault() {
	set("window_title", "Ren-Engine");

	set("window_x", "300");
	set("window_y", "70");
	set("window_width", "800");
	set("window_height", "600");

	set("max_fps", "30");
	set("max_size_textures_cache", "50");
	set("max_size_surfaces_cache", "200");
	set("count_preload_commands", "0");

	set("character_walk_fps", "4");
	set("character_run_fps", "12");

	set("character_acceleration_time", "0.5");

	set("character_walk_speed", "50");
	set("character_run_speed", "150");
}

void Config::load() {
	std::ifstream is(Utils::ROOT + "params.conf");
	if (!is.is_open()) {
		Utils::outMsg("Config::load", "Не удалось загрузить файл <" + Utils::ROOT + "params.conf>'\n"
									  "Используются настройки по-умолчанию");
		return;
	}

	while (!is.eof()) {
		String line;
		std::getline(is, line);

		size_t indexComment = line.find('#');
		String comment;
		if (indexComment != size_t(-1)) {
			comment = line.substr(indexComment + 1);

			size_t start = comment.find_first_not_of(' ');
			if (start != size_t(-1)) {
				comment.erase(0, start);
			}

			size_t end = comment.find_last_not_of(' ') + 1;
			comment.erase(end);
		}

		line.deleteAll("\t");

		if (!line) continue;
		if (line[0] == '#') {
			Utils::outMsg("Config::load", "Комментарии, начинающиеся с начала строки, являются некорректными");
			continue;
		}

		size_t indexAssign = line.find('=');
		indexComment = line.find('#');

		if (indexAssign == size_t(-1)) {
			Utils::outMsg("Config::load", "В строке <" + line + "> нет символа '='");
			continue;
		}
		if (indexComment == size_t(-1)) {
			indexComment = line.size();
		}else if (indexComment < indexAssign) {
			Utils::outMsg("Config::load", "В строке <" + line + "> символ '#', начинающий комментарий, идёт перед символом '='");
			continue;
		}

		String name = line.substr(0, indexAssign);
		size_t startName = name.find_first_not_of(' ');
		size_t endName = name.find_last_not_of(' ');
		if (startName != endName) {
			name = name.substr(startName, endName - startName + 1);
		}

		String value = line.substr(indexAssign + 1, indexComment - (indexAssign + 1));
		size_t startValue = value.find_first_not_of(' ');
		size_t endValue = value.find_last_not_of(' ');
		value = value.substr(startValue, endValue - startValue + 1);

		set(name, value, comment);
	}
}

void Config::save() {
	if (!changed) return;
	changed = false;

	auto getCountLetters = [](const String &s) {
		size_t res = 0;

		for (size_t i = 0; i < s.size(); ++i) {
			if (Utils::isFirstByte(s[i])) {
				++res;
			}
		}
		return res;
	};

	size_t maxSize = 0;
	for (size_t i = 0; i < params.size(); ++i) {
		String name = params[i].name;
		String value = params[i].value;

		size_t size = getCountLetters(name) + 3 + getCountLetters(value);//3 = getCountLetters(" = ")

		if (size > maxSize) {
			maxSize = size;
		}
	}

	std::ofstream os(Utils::ROOT + "params.conf");

	for (size_t i = 0; i < params.size(); ++i) {
		const Param &param = params[i];

		const String &name = param.name;
		const String &value = param.value;
		const String &comment = param.comment;

		os << name << " = " << value;

		if (comment) {
			size_t size = getCountLetters(name) + 3 + getCountLetters(value);//3 = getCountLetters(" = ")

			String spaces;
			spaces.resize(maxSize - size, ' ');
			os << spaces << "  # " << comment;
		}
		os << '\n';
	}
}
