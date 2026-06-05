#include "clipboard.h"

#include <SDL3/SDL_clipboard.h>

#include "utils/message.h"
#include "utils/thread_tasks.h"

std::string Clipboard::get() {
	std::string res;

	ThreadTasks::main.addAndWait([&]() {
		const char *tmp = SDL_GetClipboardText();
		res = tmp;
		SDL_free((void*)tmp);
		if (res.empty()) {
			std::string error = SDL_GetError();
			if (!error.empty()) {
				Message::outMsg("Clipboard::get", error);
			}
		}
	});

	return res;
}

bool Clipboard::set(const std::string &s) {
	bool res;

	ThreadTasks::main.addAndWait([&]() {
		if (!SDL_SetClipboardText(s.c_str())) {
			Message::outMsg("Clipboard::set", SDL_GetError());
			res = false;
		}else {
			res = true;
		}
	});

	return res;
}
