#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <type_traits>

#include "utils/image_typedefs.h"


class Utils {
private:
	static std::string formatImpl(std::string_view format, const std::string *strs, size_t n);

public:
	static std::string getVersion();

	static void setThreadName(std::string name);

	static std::string getClipboardText();
	static bool setClipboardText(const std::string &text);

	static double getTimer();
	static void sleep(double sec, bool checkInGame = true);

	static Uint32 getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop);

	static std::string md5(const std::string &str);

	static void outMsg(std::string msg, const std::string &err = "");
	static bool realOutMsg();


	//format placeholder is just %
	template<typename ...Args>
	static std::string format(std::string_view format, Args ...args) {
		constexpr size_t SIZE = sizeof...(args);
		static_assert(SIZE != 0);

		auto toString = [](auto arg) -> std::string {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (
				std::is_same_v<T, std::string>      ||
				std::is_same_v<T, std::string_view> ||
				std::is_same_v<T, const char *>
			) {
				return std::string(arg);
			}else {
				return std::to_string(arg);
			}
		};

		const std::string strArgs[SIZE] = { toString(args)... };

		return Utils::formatImpl(format, strArgs, SIZE);
	}


	//outMsg with error formatting (and mb with translating in the future?)
	template<typename ...Args>
	static void outError(const std::string &fromFunc, std::string_view format, Args ...args) {
		Utils::outMsg(fromFunc, Utils::format(format, args...));
	}
};

#endif // UTILS_H
