#pragma once
#include <Windows.h>

namespace veh {
	bool install(void* target, void* hook_fn);
	void remove();

	extern bool in_original;
	extern void* target_addr;
}
