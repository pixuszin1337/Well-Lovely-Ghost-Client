#include "syscall.h"
#include "../includes.h"

namespace {

    using fn_generic = LONG(*)(void*, void*, void*, void*);

    fn_generic g_NtQueryInformationProcess = nullptr;
    fn_generic g_NtSetInformationThread = nullptr;
    fn_generic g_NtQuerySystemInformation = nullptr;
    fn_generic g_NtClose = nullptr;

    uint32_t read_ssn(const char* func_name) {
        auto addr = reinterpret_cast<uint8_t*>(
            GetProcAddress(GetModuleHandleW(L"ntdll.dll"), func_name));
        if (!addr) return 0xFFFFFFFF;

        if (addr[0] == 0x4C && addr[1] == 0x8B && addr[2] == 0xD1 &&
            addr[3] == 0xB8)
            return *reinterpret_cast<uint32_t*>(addr + 4);

        for (int i = 0; i < 64; i++) {
            if (addr[i] == 0x0F && addr[i + 1] == 0x05) {
                for (int j = i - 1; j >= 0; j--) {
                    if (addr[j] == 0xB8)
                        return *reinterpret_cast<uint32_t*>(addr + j + 1);
                }
            }
        }

        return 0xFFFFFFFF;
    }

    fn_generic create_stub(uint32_t ssn) {
        if (ssn == 0xFFFFFFFF) return nullptr;

        auto mem = reinterpret_cast<uint8_t*>(
            VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        if (!mem) return nullptr;

        int i = 0;
        mem[i++] = 0x4C; mem[i++] = 0x8B; mem[i++] = 0xD1;
        mem[i++] = 0xB8;
        *reinterpret_cast<uint32_t*>(mem + i) = ssn; i += 4;
        mem[i++] = 0x0F; mem[i++] = 0x05;
        mem[i++] = 0xC3;

        DWORD old;
        VirtualProtect(mem, 64, PAGE_EXECUTE_READ, &old);
        return reinterpret_cast<fn_generic>(mem);
    }

}

void syscall::init() {
    g_NtQueryInformationProcess = create_stub(
        read_ssn(xorstr_("NtQueryInformationProcess")));
    g_NtSetInformationThread = create_stub(
        read_ssn(xorstr_("NtSetInformationThread")));
    g_NtQuerySystemInformation = create_stub(
        read_ssn(xorstr_("NtQuerySystemInformation")));
    g_NtClose = create_stub(
        read_ssn(xorstr_("NtClose")));
}

LONG syscall::nt_query_information_process(HANDLE process, ULONG info_class,
    PVOID info, ULONG info_len, PULONG ret_len) {
    if (!g_NtQueryInformationProcess) return -1;
    return g_NtQueryInformationProcess(process,
        reinterpret_cast<void*>(static_cast<uintptr_t>(info_class)),
        info,
        reinterpret_cast<void*>(static_cast<uintptr_t>(info_len)));
}

LONG syscall::nt_set_information_thread(HANDLE thread, ULONG info_class,
    PVOID info, ULONG info_len) {
    if (!g_NtSetInformationThread) return -1;
    return g_NtSetInformationThread(thread,
        reinterpret_cast<void*>(static_cast<uintptr_t>(info_class)),
        info,
        reinterpret_cast<void*>(static_cast<uintptr_t>(info_len)));
}

LONG syscall::nt_query_system_information(ULONG info_class,
    PVOID info, ULONG info_len, PULONG ret_len) {
    if (!g_NtQuerySystemInformation) return -1;
    return g_NtQuerySystemInformation(
        reinterpret_cast<void*>(static_cast<uintptr_t>(info_class)),
        info,
        reinterpret_cast<void*>(static_cast<uintptr_t>(info_len)),
        reinterpret_cast<void*>(ret_len));
}

LONG syscall::nt_close(HANDLE handle) {
    if (!g_NtClose) return -1;
    return g_NtClose(handle, nullptr, nullptr, nullptr);
}
