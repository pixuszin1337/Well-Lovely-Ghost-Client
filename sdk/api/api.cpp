#include "api.h"

#define RESOLVE(mod, name) \
    reinterpret_cast<decltype(api::p##name)>( \
        api::resolve_by_hash(mod, api::ct_hash(#name)))

namespace api {
    BOOL(WINAPI* pIsDebuggerPresent)() = nullptr;
    BOOL(WINAPI* pCheckRemoteDebuggerPresent)(HANDLE, PBOOL) = nullptr;
    HWND(WINAPI* pFindWindowW)(LPCWSTR, LPCWSTR) = nullptr;
    HMODULE(WINAPI* pGetModuleHandleW)(LPCWSTR) = nullptr;
    int(WINAPI* pGetSystemMetrics)(int) = nullptr;
    HANDLE(WINAPI* pOpenProcess)(DWORD, BOOL, DWORD) = nullptr;
    BOOL(WINAPI* pCloseHandle)(HANDLE) = nullptr;
    BOOL(WINAPI* pK32EnumDeviceDrivers)(LPVOID*, DWORD, LPDWORD) = nullptr;
    DWORD(WINAPI* pK32GetDeviceDriverBaseNameW)(LPVOID, LPWSTR, DWORD) = nullptr;
    DWORD(WINAPI* pK32GetProcessImageFileNameW)(HANDLE, LPWSTR, DWORD) = nullptr;
    HANDLE(WINAPI* pGetCurrentProcess)() = nullptr;
    DWORD(WINAPI* pGetCurrentProcessId)() = nullptr;
}

void api::init() {
    HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
    HMODULE u32 = GetModuleHandleW(L"user32.dll");
    if (!u32) u32 = LoadLibraryW(L"user32.dll");

    pIsDebuggerPresent = RESOLVE(k32, IsDebuggerPresent);
    pCheckRemoteDebuggerPresent = RESOLVE(k32, CheckRemoteDebuggerPresent);
    pGetModuleHandleW = RESOLVE(k32, GetModuleHandleW);
    pOpenProcess = RESOLVE(k32, OpenProcess);
    pCloseHandle = RESOLVE(k32, CloseHandle);
    pGetCurrentProcess = RESOLVE(k32, GetCurrentProcess);
    pGetCurrentProcessId = RESOLVE(k32, GetCurrentProcessId);
    pK32EnumDeviceDrivers = RESOLVE(k32, K32EnumDeviceDrivers);
    pK32GetDeviceDriverBaseNameW = RESOLVE(k32, K32GetDeviceDriverBaseNameW);
    pK32GetProcessImageFileNameW = RESOLVE(k32, K32GetProcessImageFileNameW);

    if (u32) {
        pFindWindowW = RESOLVE(u32, FindWindowW);
        pGetSystemMetrics = RESOLVE(u32, GetSystemMetrics);
    }
}

#undef RESOLVE
