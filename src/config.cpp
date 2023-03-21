#include "config.h"

#include <cstring>
#include <fstream>
#include <vector>

#include <SDL2/SDL_hints.h>

#include "utils/algo.h"
#include "utils/string.h"
#include "utils/utils.h"


struct Param {
	std::string name;
	std::string value;
	std::string comment;
};

static std::vector<Param> params;
static bool initing = false;
static bool loading = false;
static bool changed = false;

static void setDefault();
static void load();

static const char *pathOrig = "./params.conf";
static const char *pathUser = "../var/params.conf";


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

	Utils::outMsg("Config::get", "Unknown param <" + name + ">");
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
		Utils::outMsg("Config::set", "Unknown param <" + name + ">");
	}
}

static void setDefault() {
	Config::set("window_title", "Ren-Engine");
	Config::set("window_icon", "None");

	Config::set("window_x", "None");
	Config::set("window_y", "None");
	Config::set("window_width", "1200");
	Config::set("window_height", "675");
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
	Config::set("max_size_surfaces_cache", "150");
	Config::set("count_preload_commands", "3");
}

static void load() {
	std::ifstream is(pathUser, std::ios::binary);
	if (!is.is_open()) {
		is.open(pathOrig, std::ios::binary);
		if (!is.is_open()) {
			Utils::outMsg("Config::load", "Failed to load file <" + std::string(pathOrig) + ">\n"
			                              "Using default settings");
			return;
		}
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
			Utils::outMsg("Config::load", "Comment at the begining of line is invalid");
			continue;
		}

		size_t indexAssign = line.find('=');
		if (indexAssign == size_t(-1)) {
			Utils::outMsg("Config::load", "Expected symbol '=' in <" + line + ">");
			continue;
		}
		if (indexComment == size_t(-1)) {
			indexComment = line.size();
		}else if (indexComment < indexAssign) {
			Utils::outMsg("Config::load", "First symbol '=' in <" + line + "> is commented");
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

	std::ofstream os(pathUser, std::ios::binary);

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


void Config::setDefaultScaleQuality() {
	std::string value = Config::get("scale_quality");
	Config::setScaleQuality(value);
}
bool Config::setScaleQuality(std::string value) {
	bool ok = true;
	if (value != "0" && value != "1" && value != "2") {
		Utils::outMsg("Config::setScaleQuality", "Expected 0, 1 or 2, got <" + value + ">");
		value = "0";
		ok = false;
	}
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, value.c_str()) == SDL_FALSE) {
		Utils::outMsg("SDL_SetHint", "Failed to set scale quality");
		ok = false;
	}
	return ok;
}
std::string Config::getScaleQuality() {
	return SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
}
