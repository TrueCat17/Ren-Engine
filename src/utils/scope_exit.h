#ifndef SCOPE_EXIT_H
#define SCOPE_EXIT_H

#include <utility>

template<typename T>
struct ScopeExit {
private:
	T func;

public:
	bool enable = true;

	ScopeExit(const ScopeExit &) = delete;
	ScopeExit& operator=(const ScopeExit &) = delete;

	ScopeExit(ScopeExit &&) = delete;
	ScopeExit& operator=(ScopeExit &&) = delete;

	ScopeExit(T&& func): func(std::move(func)) {}
	~ScopeExit() { if (enable) func(); }
};

#endif // SCOPE_EXIT_H
