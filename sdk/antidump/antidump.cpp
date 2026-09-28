#include "antidump.h"
#include "../includes.h"

namespace {

    void zero_section(uint8_t* base, PIMAGE_SECTION_HEADER section) {
        DWORD old;
        if (VirtualProtect(base + section->VirtualAddress,
            section->Misc.VirtualSize, PAGE_READWRITE, &old)) {
            SecureZeroMemory(base + section->VirtualAddress,
                section->Misc.VirtualSize);
            VirtualProtect(base + section->VirtualAddress,
                section->Misc.VirtualSize, old, &old);
        }
    }

    bool name_matches(const BYTE* sec_name, const char* target) {
        for (int i = 0; i < 8 && target[i]; i++) {
            if (sec_name[i] != static_cast<BYTE>(target[i]))
                return false;
        }
        return true;
    }

    void wipe_expendable_sections(uint8_t* base, PIMAGE_NT_HEADERS nt) {
        auto section = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++) {
            if (section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
                zero_section(base, section);

            if (name_matches(section->Name, ".reloc"))
                zero_section(base, section);

            if (name_matches(section->Name, ".rsrc"))
                zero_section(base, section);
        }
    }

    void corrupt_data_directories(uint8_t* base, PIMAGE_NT_HEADERS nt) {
        DWORD old;
        auto& opt = nt->OptionalHeader;
        if (!VirtualProtect(&opt.DataDirectory, sizeof(opt.DataDirectory),
            PAGE_READWRITE, &old))
            return;

        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = 0;
        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = 0;
        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = 0;
        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = 0;
        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress = 0;
        opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size = 0;

        VirtualProtect(&opt.DataDirectory, sizeof(opt.DataDirectory), old, &old);
    }

}

void antidump::apply(HINSTANCE dll_base) {
    auto base = reinterpret_cast<uint8_t*>(dll_base);
    auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;

    auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;

    wipe_expendable_sections(base, nt);
    corrupt_data_directories(base, nt);
}
