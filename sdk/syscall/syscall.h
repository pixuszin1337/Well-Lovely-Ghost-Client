#pragma once
#include <windows.h>
#include <cstdint>

namespace syscall {
    void init();

    LONG nt_query_information_process(HANDLE process, ULONG info_class,
        PVOID info, ULONG info_len, PULONG ret_len);
    LONG nt_set_information_thread(HANDLE thread, ULONG info_class,
        PVOID info, ULONG info_len);
    LONG nt_query_system_information(ULONG info_class,
        PVOID info, ULONG info_len, PULONG ret_len);
    LONG nt_close(HANDLE handle);
}
