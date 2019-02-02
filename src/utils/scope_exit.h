#ifndef SCOPE_EXIT_H
#define SCOPE_EXIT_H

#include <bits/std_function.h>

struct ScopeExit {
	std::function<void()> func;

	ScopeExit(const std::function<void()>& func): func(func) {}
	~ScopeExit() { if (func) func(); }
};

#endif // SCOPE_EXIT_H
