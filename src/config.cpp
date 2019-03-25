#include "config.h"

#include <cstring>
#include <fstream>

#include "utils/algo.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::vector<Param> params;
static bool initing = false;
static bool loading = false;
static bool changed = false;

static void setDefault();
static void load();


void Config::init() {
	initing = true;
	setDefault();
	initing = false;

	loading = true;
	load();
	loading = false;
}

std::string Config::get(const std::string &name) {
	for (const Param &param : params) {
		if (param.name == name) return param.value;
	}

	Utils::outMsg("Config::get", "Param <" + name + "> there is not");
	return "";
}

void Config::set(const std::string &name, const std::string &value, const std::string &comment) {
	for (Param &param : params) {
		if (param.name != name) continue;

		bool assigned = false;
		if (param.value != value) {
			param.value = value;
			assigned = true;
		}
		if (!comment.empty() && param.comment != comment) {
			param.comment = comment;
			assigned = true;
		}

		if (!initing && !loading && assigned) {
			changed = true;
		}
		return;
	}

	if (initing) {
		params.push_back({name, value, comment});
	}else {
		Utils::outMsg("Config::set", "Param <" + name + "> there is not");
	}
}

static void setDefault() {
	Config::set("window_title", "Ren-Engine");
	Config::set("window_icon", "None");

	Config::set("window_x", "300");
	Config::set("window_y", "70");
	Config::set("window_width", "800");
	Config::set("window_height", "600");
	Config::set("window_w_div_h", "1.777");
	Config::set("window_fullscreen", "False");

	Config::set("mouse_usual", "None");
	Config::set("mouse_btn", "None");
	Config::set("mouse_hide_time", "0");

	Config::set("scale_quality", "0");
	Config::set("software_renderer", "False");
	Config::set("fast_opengl", "True");
	Config::set("check_fast_opengl_errors", "False");
	Config::set("opengl_vsync", "False");
	Config::set("max_fps", "30");

	Config::set("max_size_textures_cache", "50");
	Config::set("max_size_surfaces_cache", "200");
	Config::set("count_preload_commands", "3");
}

static void load() {
	std::ifstream is("params.conf");
	if (!is.is_open()) {
		Utils::outMsg("Config::load", "Could not to load file <resources/params.conf>\n"
		                              "Using default settings");
		return;
	}

	while (!is.eof()) {
		std::string line;
		std::getline(is, line);

		size_t indexComment = line.find('#');
		std::string comment;
		if (indexComment != size_t(-1)) {
			comment = line.substr(indexComment + 1);

			size_t start = comment.find_first_not_of(' ');
			if (start != size_t(-1)) {
				comment.erase(0, start);
			}

			size_t end = comment.find_last_not_of(' ') + 1;
			comment.erase(end);
		}

		String::deleteAll(line, "\t");

		if (line.empty()) continue;
		if (line[0] == '#') {
			Utils::outMsg("Config::load", "Comment at the begining of the line is invalid");
			continue;
		}

		size_t indexAssign = line.find('=');
		if (indexAssign == size_t(-1)) {
			Utils::outMsg("Config::load", "In the line <" + line + "> there is no symbol '='");
			continue;
		}
		if (indexComment == size_t(-1)) {
			indexComment = line.size();
		}else if (indexComment < indexAssign) {
			Utils::outMsg("Config::load", "In the line <" + line + "> first symbol '=' is commented");
			continue;
		}

		std::string name = line.substr(0, indexAssign);
		size_t startName = name.find_first_not_of(' ');
		size_t endName = name.find_last_not_of(' ');
		if (startName != endName) {
			name = name.substr(startName, endName - startName + 1);
		}

		std::string value = line.substr(indexAssign + 1, indexComment - (indexAssign + 1));
		size_t startValue = value.find_first_not_of(' ');
		size_t endValue = value.find_last_not_of(' ');
		value = value.substr(startValue, endValue - startValue + 1);

		Config::set(name, value, comment);
	}
}

void Config::save() {
	if (!changed) return;
	changed = false;

	auto getCountLetters = [](const std::string &s) {
		size_t res = 0;
		for (size_t i = 0; i < s.size(); ++i) {
			if (Algo::isFirstByte(s[i])) {
				++res;
			}
		}
		return res;
	};

	size_t maxSize = 0;
	for (const Param &param : params) {
		const std::string &name = param.name;
		const std::string &value = param.value;

		size_t size = getCountLetters(name) + strlen(" = ") + getCountLetters(value);

		if (size > maxSize) {
			maxSize = size;
		}
	}

	std::ofstream os("params.conf");

	for (const Param &param : params) {
		const std::string &name = param.name;
		const std::string &value = param.value;
		const std::string &comment = param.comment;

		os << name << " = " << value;

		if (!comment.empty()) {
			size_t size = getCountLetters(name) + strlen(" = ") + getCountLetters(value);

			std::string spaces;
			spaces.resize(maxSize - size, ' ');
			os << spaces << "  # " << comment;
		}
		os << '\n';
	}
}
