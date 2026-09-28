#include "input_hook.h"
#include <atomic>
#include <thread>

namespace {
    std::atomic<bool> g_lmb_down{ false };
    std::atomic<bool> g_running{ false };
    DWORD g_thread_id = 0;
    HHOOK g_hook = nullptr;
    std::thread g_thread;

    LRESULT CALLBACK mouse_proc(int code, WPARAM wp, LPARAM lp) {
        if (code >= 0) {
            auto* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lp);
            if (!(info->flags & LLMHF_INJECTED)) {
                if (wp == WM_LBUTTONDOWN)
                    g_lmb_down = true;
                else if (wp == WM_LBUTTONUP)
                    g_lmb_down = false;
            }
        }
        return CallNextHookEx(g_hook, code, wp, lp);
    }

    void pump() {
        g_hook = SetWindowsHookExW(WH_MOUSE_LL, mouse_proc, nullptr, 0);
        if (!g_hook) return;

        g_thread_id = GetCurrentThreadId();

        MSG msg;
        while (g_running) {
            if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) break;
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            } else {
                MsgWaitForMultipleObjects(0, nullptr, FALSE, 50, QS_ALLINPUT);
            }
        }

        UnhookWindowsHookEx(g_hook);
        g_hook = nullptr;
        g_thread_id = 0;
    }
}

void input_hook::start() {
    if (g_running.exchange(true)) return;
    g_thread = std::thread(pump);
}

void input_hook::stop() {
    g_running = false;
    if (g_thread_id)
        PostThreadMessageW(g_thread_id, WM_QUIT, 0, 0);
    if (g_thread.joinable())
        g_thread.join();
}

bool input_hook::is_lmb_down() {
    return g_lmb_down;
}
