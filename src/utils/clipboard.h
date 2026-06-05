#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <string>

class Clipboard {
public:
	static std::string get();
	static bool set(const std::string &s);
};

#endif // CLIPBOARD_H
