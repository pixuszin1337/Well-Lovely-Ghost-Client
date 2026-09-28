#pragma once
#include <windows.h>
#include <cstdint>
#include <Psapi.h>

namespace api {

    constexpr uint32_t ct_hash(const char* s) {
        uint32_t h = 0x811c9dc5;
        for (; *s; ++s) {
            h ^= static_cast<uint8_t>(*s);
            h *= 0x01000193;
        }
        return h;
    }

    inline FARPROC resolve_by_hash(HMODULE mod, uint32_t target_hash) {
        auto base = reinterpret_cast<uint8_t*>(mod);
        auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
        auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

        auto& exp_entry = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (!exp_entry.VirtualAddress) return nullptr;

        auto exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(base + exp_entry.VirtualAddress);
        auto names = reinterpret_cast<uint32_t*>(base + exports->AddressOfNames);
        auto ordinals = reinterpret_cast<uint16_t*>(base + exports->AddressOfNameOrdinals);
        auto funcs = reinterpret_cast<uint32_t*>(base + exports->AddressOfFunctions);

        for (DWORD i = 0; i < exports->NumberOfNames; i++) {
            auto name = reinterpret_cast<const char*>(base + names[i]);
            uint32_t h = 0x811c9dc5;
            for (auto p = name; *p; ++p) {
                h ^= static_cast<uint8_t>(*p);
                h *= 0x01000193;
            }
            if (h == target_hash)
                return reinterpret_cast<FARPROC>(base + funcs[ordinals[i]]);
        }
        return nullptr;
    }

    void init();

    extern BOOL(WINAPI* pIsDebuggerPresent)();
    extern BOOL(WINAPI* pCheckRemoteDebuggerPresent)(HANDLE, PBOOL);
    extern HWND(WINAPI* pFindWindowW)(LPCWSTR, LPCWSTR);
    extern HMODULE(WINAPI* pGetModuleHandleW)(LPCWSTR);
    extern int(WINAPI* pGetSystemMetrics)(int);
    extern HANDLE(WINAPI* pOpenProcess)(DWORD, BOOL, DWORD);
    extern BOOL(WINAPI* pCloseHandle)(HANDLE);
    extern BOOL(WINAPI* pK32EnumDeviceDrivers)(LPVOID*, DWORD, LPDWORD);
    extern DWORD(WINAPI* pK32GetDeviceDriverBaseNameW)(LPVOID, LPWSTR, DWORD);
    extern DWORD(WINAPI* pK32GetProcessImageFileNameW)(HANDLE, LPWSTR, DWORD);
    extern HANDLE(WINAPI* pGetCurrentProcess)();
    extern DWORD(WINAPI* pGetCurrentProcessId)();

}
