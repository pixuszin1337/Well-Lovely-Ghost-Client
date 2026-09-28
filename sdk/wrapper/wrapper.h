#pragma once

namespace wrapper {

	inline HANDLE spawn_thread(LPTHREAD_START_ROUTINE routine) {
		return CreateThread(0, 0, routine, 0, 0, 0);
	}

	const inline void dbg_log(std::string msg) {
		OutputDebugStringA(msg.c_str());
	}

	const inline bool handle_issue(const char* name, void* ptr)
	{

		const auto s_formatting = xorstr_("%s not intialized, value -> 0x%X");

		if (!ptr)
		{
			size_t len = std::snprintf(nullptr, 0, s_formatting, name, ptr) + 1;

			if (len <= 0)
				return true;

			auto buffer = std::make_unique<char[]>(len);
			std::snprintf(buffer.get(), len, s_formatting, name, ptr);
			dbg_log(std::string(buffer.get(), buffer.get() + len - 1));

			return true;
		}

		return false;
	}

	inline void* get_module_handle(const char* name) {
		auto h_module = GetModuleHandleA(name);

		if (handle_issue(name, h_module))
			std::exit(0);

		return h_module;
	}

	inline HWND find_window(const char* name)
	{
		auto res = BADLION_CLIENT ? FindWindowA(xorstr_("LWJGL"), nullptr) : FindWindowA(nullptr, name);

		if (handle_issue(name, res))
			std::exit(0);

		return res;
	}

	inline RECT get_window_rect(const char* name)
	{
		RECT res;
		GetWindowRect(find_window(name), &res);

		return res;
	}

	inline void* get_proc_address(const char* name, void* handle)
	{
		auto p_procedure = GetProcAddress(reinterpret_cast<HMODULE>(handle), name);

		if (handle_issue(name, p_procedure))
			std::exit(0);

		return p_procedure;
	}

	inline short get_async_keystate(int key)
	{
		return GetAsyncKeyState(key);
	}

	struct c_context {
		HDC m_hdc_devicectx;
		HGLRC m_glrenderctx;
		HGLRC m_oglrenderctx;
	};

}
