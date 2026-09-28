#pragma once
#include <windows.h>

namespace input_hook {
    void start();
    void stop();
    bool is_lmb_down();
}
