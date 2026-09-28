#include "../sdk/includes.h"
#include "../main/welllovely.h"

#include <cstdio>

static HINSTANCE g_dll_base = nullptr;
static PVOID g_crash_handler = nullptr;

static LONG CALLBACK crash_handler(EXCEPTION_POINTERS* ep)
{
	if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;

	auto fault_addr = ep->ExceptionRecord->NumberParameters >= 2
		? ep->ExceptionRecord->ExceptionInformation[1] : 0;
	if (fault_addr < 0x10000)
		return EXCEPTION_CONTINUE_SEARCH;

	char appdata[MAX_PATH]{};
	if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) == 0)
		return EXCEPTION_CONTINUE_SEARCH;

	char dir[MAX_PATH];
	snprintf(dir, MAX_PATH, "%s\\WellLovely", appdata);
	CreateDirectoryA(dir, nullptr);

	char path[MAX_PATH];
	snprintf(path, MAX_PATH, "%s\\crash.log", dir);

	FILE* f = nullptr;
	if (fopen_s(&f, path, "a") != 0 || !f)
		return EXCEPTION_CONTINUE_SEARCH;

	auto ctx = ep->ContextRecord;
	auto rec = ep->ExceptionRecord;

	HMODULE faulting_mod = nullptr;
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)ctx->Rip, &faulting_mod);

	char mod_name[MAX_PATH] = "unknown";
	if (faulting_mod)
		GetModuleFileNameA(faulting_mod, mod_name, MAX_PATH);

	DWORD64 offset = faulting_mod ? (ctx->Rip - (DWORD64)faulting_mod) : 0;

	fprintf(f, "=== CRASH tid=%lu tick=%llu ===\n",
		GetCurrentThreadId(), (unsigned long long)GetTickCount64());
	fprintf(f, "ACCESS_VIOLATION %s at 0x%llX\n",
		rec->NumberParameters >= 1 ?
			(rec->ExceptionInformation[0] == 0 ? "READ" :
			 rec->ExceptionInformation[0] == 1 ? "WRITE" : "EXECUTE") : "?",
		rec->NumberParameters >= 2 ? (unsigned long long)rec->ExceptionInformation[1] : 0ULL);
	fprintf(f, "RIP: 0x%llX  (%s + 0x%llX)\n",
		(unsigned long long)ctx->Rip, mod_name, (unsigned long long)offset);
	fprintf(f, "DLL base: 0x%llX  RIP-DLL: 0x%llX\n",
		(unsigned long long)g_dll_base,
		g_dll_base ? (unsigned long long)(ctx->Rip - (DWORD64)g_dll_base) : 0ULL);
	fprintf(f, "RSP: 0x%llX  RBP: 0x%llX\n",
		(unsigned long long)ctx->Rsp, (unsigned long long)ctx->Rbp);
	fprintf(f, "RAX: 0x%llX  RBX: 0x%llX  RCX: 0x%llX  RDX: 0x%llX\n",
		(unsigned long long)ctx->Rax, (unsigned long long)ctx->Rbx,
		(unsigned long long)ctx->Rcx, (unsigned long long)ctx->Rdx);
	fprintf(f, "RSI: 0x%llX  RDI: 0x%llX\n",
		(unsigned long long)ctx->Rsi, (unsigned long long)ctx->Rdi);
	fprintf(f, "R8:  0x%llX  R9:  0x%llX  R10: 0x%llX  R11: 0x%llX\n",
		(unsigned long long)ctx->R8, (unsigned long long)ctx->R9,
		(unsigned long long)ctx->R10, (unsigned long long)ctx->R11);

	fprintf(f, "Stack (top 20 qwords from RSP):\n");
	DWORD64* sp = (DWORD64*)ctx->Rsp;
	for (int i = 0; i < 20; i++)
	{
		__try {
			DWORD64 val = sp[i];
			fprintf(f, "  [RSP+0x%02X] 0x%llX", i * 8, (unsigned long long)val);
			HMODULE sm = nullptr;
			if (GetModuleHandleExA(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCSTR)val, &sm) && sm)
			{
				char sn[MAX_PATH] = {};
				GetModuleFileNameA(sm, sn, MAX_PATH);
				fprintf(f, "  (%s+0x%llX)", sn, (unsigned long long)(val - (DWORD64)sm));
			}
			fprintf(f, "\n");
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			fprintf(f, "  [RSP+0x%02X] <inaccessible>\n", i * 8);
			break;
		}
	}

	fprintf(f, "===\n\n");
	fclose(f);

	return EXCEPTION_CONTINUE_SEARCH;
}

void start_welllovely_thread();

int __stdcall DllMain(
    HINSTANCE hinstDLL,
    unsigned long     fdwReason,
    void* lpvReserved
)   {

    if (fdwReason != DLL_PROCESS_ATTACH)
        return false;

    g_dll_base = hinstDLL;

    wrapper::spawn_thread(reinterpret_cast<LPTHREAD_START_ROUTINE>(start_welllovely_thread));

    return true;
}

void start_welllovely_thread() {

    g_crash_handler = AddVectoredExceptionHandler(0, crash_handler);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    welllovely::instance = std::make_unique<welllovely::c_welllovely>();

    if (!welllovely::instance->attach())
        std::exit(0);

    welllovely::instance->run();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    welllovely::instance->dispose();
}
