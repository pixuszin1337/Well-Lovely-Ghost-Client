#pragma once
#include <windows.h>
#include <cstdint>

namespace integrity {
    void snapshot();
    bool verify();
    void start();
}
