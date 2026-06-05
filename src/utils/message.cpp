#include "message.h"

#include <mutex>
#include <set>
#include <vector>

#include "SDL3/SDL_messagebox.h"

#include "logger.h"
#include "gui/screen/key.h"

#include "utils/stage.h"
#include "utils/string.h"
#include "utils/thread_tasks.h"


static std::mutex msgGuard;
static std::set<std::string> msgErrors;
static bool msgCloseAll = false;

static const SDL_MessageBoxButtonData buttons[2] = {
    { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Ok" },
    { 0, 1, "Close All" }
};

void Message::outMsg(std::string msg, const std::string &err) {
	std::lock_guard g(msgGuard);

	if (!err.empty()) {
		msg += " Error:\n" + err;
	}

	const size_t maxLineSize = 100;
	const size_t maxLineCount = 30;
	std::vector<std::string> lines = String::split(msg, "\n");
	for (size_t i = 0; i < lines.size() && i < maxLineCount; ++i) {
		const std::string &line = lines[i];
		if (line.size() <= maxLineSize) continue;

		size_t space = line.find_last_of(' ', maxLineSize - 1);
		if (space <= maxLineSize / 2 || space == size_t(-1)) {//small string or no spaces?
			space = maxLineSize;//word-wrap in the middle of a word
		}

		lines.insert(lines.begin() + long(i) + 1, line.substr(space));
		lines[i].erase(space);//no just "line" (vector mb reallocated on prev line)
	}
	if (lines.size() > maxLineCount) {
		lines.erase(lines.begin() + maxLineCount - 1, lines.end());
		lines.push_back("..");
	}
	msg = String::join(lines, "\n");

	if (msg.empty() || msgErrors.count(msg)) return;

	Logger::log(msg + "\n\n");

	msgErrors.insert(msg);

	if (msgCloseAll) return;

	ThreadTasks::main.addAndWait([&]() {
		SDL_MessageBoxData data;
		data.flags = err.empty() ? SDL_MESSAGEBOX_WARNING : SDL_MESSAGEBOX_ERROR;
		data.window = Stage::window;
		data.title = "Message";
		data.message = msg.c_str();
		data.numbuttons = 2;
		data.buttons = buttons;
		data.colorScheme = nullptr;

		Key::resetPressed();

		int res = 0;
		if (!SDL_ShowMessageBox(&data, &res)) {
			printf("%s\n", msg.c_str());
			printf("%s\n", SDL_GetError());
		}

		Key::resetPressed();

		if (res == 1) {
			msgCloseAll = true;
		}
	});
}


std::string Message::formatImpl(std::string_view format, const std::string *strs, size_t n) {
	std::string res;
	size_t msgSize = format.size() - n;
	for (size_t i = 0; i < n; ++i) {
		msgSize += strs[i].size();
	}
	res.reserve(msgSize);

	size_t start = 0;
	size_t index = format.find('%');
	size_t strIndex = 0;
	while (index != size_t(-1)) {
		res += format.substr(start, index - start);
		start = index + 1;
		index = format.find('%', start);

		if (strIndex < n) {
			res += strs[strIndex++];
		}else {
			Message::outMsg("Message::formatImpl", "Need more args to format message");
			break;
		}
	}
	res += format.substr(start);

	if (strIndex != n) {
		Message::outMsg("Message::formatImpl", "Need more placeholders to format message");
		res += "\n(";
		while (strIndex < n) {
			res += strs[strIndex++];
			if (strIndex != n) {
				res += ", ";
			}
		}
		res += ')';
	}

	return res;
}