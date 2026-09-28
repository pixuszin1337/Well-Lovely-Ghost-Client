#pragma once

namespace stealth {

	inline void erase_pe_header(void* base)
	{
		if (!base)
			return;

		auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return;

		auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
			reinterpret_cast<BYTE*>(base) + dos->e_lfanew);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return;

		DWORD old_protect = 0;
		DWORD header_size = nt->OptionalHeader.SizeOfHeaders;

		VirtualProtect(base, header_size, PAGE_READWRITE, &old_protect);
		SecureZeroMemory(base, header_size);
		VirtualProtect(base, header_size, old_protect, &old_protect);
	}

	inline void clear_debug_flags()
	{
#ifdef _WIN64
		auto peb = reinterpret_cast<BYTE*>(__readgsqword(0x60));
#else
		auto peb = reinterpret_cast<BYTE*>(__readfsdword(0x30));
#endif
		peb[0x02] = 0;

#ifdef _WIN64
		*reinterpret_cast<DWORD*>(peb + 0xBC) = 0;
#else
		*reinterpret_cast<DWORD*>(peb + 0x68) = 0;
#endif
	}

	inline void hide_thread()
	{
		using fn_t = LONG(WINAPI*)(HANDLE, ULONG, PVOID, ULONG);
		auto ntdll = GetModuleHandleA(xorstr_("ntdll.dll"));
		if (!ntdll)
			return;
		auto fn = reinterpret_cast<fn_t>(
			GetProcAddress(ntdll, xorstr_("NtSetInformationThread")));
		if (fn)
			fn(GetCurrentThread(), 0x11, nullptr, 0);
	}

	inline void apply_all(void* dll_base = nullptr)
	{
		clear_debug_flags();
		hide_thread();
		if (dll_base)
			erase_pe_header(dll_base);
	}
}
