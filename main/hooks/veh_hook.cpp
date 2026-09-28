#include "veh_hook.h"
#include <TlHelp32.h>

namespace veh {

bool in_original = false;
void* target_addr = nullptr;

static void* g_hook_fn = nullptr;
static PVOID g_handler = nullptr;

static LONG CALLBACK exception_handler(EXCEPTION_POINTERS* ep)
{
	if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
		return EXCEPTION_CONTINUE_SEARCH;

	auto ctx = ep->ContextRecord;

	if (ctx->Rip == reinterpret_cast<DWORD64>(target_addr))
	{
		if (in_original)
		{
			ctx->Dr0 = 0;
			ctx->Dr7 &= ~1ULL;
			ctx->EFlags |= 0x100;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		ctx->Rip = reinterpret_cast<DWORD64>(g_hook_fn);
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	if (!(ctx->Dr7 & 1) && in_original)
	{
		ctx->Dr0 = reinterpret_cast<DWORD64>(target_addr);
		ctx->Dr7 |= 1;
		ctx->EFlags &= ~0x100ULL;
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

static void set_dr0_all_threads(void* addr, bool enable)
{
	DWORD pid = GetCurrentProcessId();
	DWORD self = GetCurrentThreadId();
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return;

	THREADENTRY32 te{};
	te.dwSize = sizeof(te);
	if (Thread32First(snap, &te))
	{
		do {
			if (te.th32OwnerProcessID != pid || te.th32ThreadID == self)
				continue;

			HANDLE th = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
			if (!th)
				continue;

			SuspendThread(th);

			CONTEXT ctx{};
			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			GetThreadContext(th, &ctx);

			if (enable)
			{
				ctx.Dr0 = reinterpret_cast<DWORD64>(addr);
				ctx.Dr7 |= 1;
				ctx.Dr7 &= ~(3ULL << 16);
				ctx.Dr7 &= ~(3ULL << 18);
			}
			else
			{
				ctx.Dr0 = 0;
				ctx.Dr7 &= ~1ULL;
			}

			SetThreadContext(th, &ctx);
			ResumeThread(th);
			CloseHandle(th);
		} while (Thread32Next(snap, &te));
	}
	CloseHandle(snap);
}

bool install(void* target, void* hook_fn)
{
	target_addr = target;
	g_hook_fn = hook_fn;

	g_handler = AddVectoredExceptionHandler(1, exception_handler);
	if (!g_handler)
		return false;

	set_dr0_all_threads(target, true);
	return true;
}

void remove()
{
	if (g_handler)
	{
		set_dr0_all_threads(nullptr, false);
		RemoveVectoredExceptionHandler(g_handler);
		g_handler = nullptr;
	}
	target_addr = nullptr;
	g_hook_fn = nullptr;
	in_original = false;
}

}
