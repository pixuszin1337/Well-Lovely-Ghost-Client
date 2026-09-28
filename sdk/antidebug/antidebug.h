#pragma once
#include <windows.h>
#include <atomic>

namespace antidebug {
    void start();
    extern std::atomic<bool> detected;
}
