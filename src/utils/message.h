#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

class Message {
private:
	static std::string formatImpl(std::string_view format, const std::string *strs, size_t n);

public:
	static void outMsg(std::string msg, const std::string &err = "");


	//format placeholder is just %
	template<typename ...Args>
	static std::string format(std::string_view format, const Args &...args) {
		constexpr size_t SIZE = sizeof...(args);
		static_assert(SIZE != 0);

		auto toString = [](const auto &arg) -> std::string {
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

		return Message::formatImpl(format, strArgs, SIZE);
	}


	//outMsg with error formatting (and mb with translating in the future?)
	template<typename ...Args>
	static void outError(const std::string &fromFunc, std::string_view format, const Args &...args) {
		Message::outMsg(fromFunc, Message::format(format, args...));
	}
};

#endif // MESSAGE_H
