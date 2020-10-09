#ifndef TRANSLATION_H
#define TRANSLATION_H

#include "parser/node.h"


class Translation {
public:
	static void init();
	static void addLang(const std::string &lang);

	static const std::string &getLang();
	static void setLang(const std::string &lang);
	static void enable();

	static const std::string& get(const std::string &str);
	static PyObject* getKnownLanguages();
};

#endif // TRANSLATION_H
