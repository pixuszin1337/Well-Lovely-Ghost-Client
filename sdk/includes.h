#pragma once

#include <Windows.h>
#include <thread>
#include <memory>
#include <chrono>
#include <thread>
#include <string>

#include <unordered_map>
#include <mutex>
#include <functional>

#include <jni.h>
#include <jvmti.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline auto BADLION_CLIENT = false;
inline auto CUSTOM_CLIENT = false;

inline constexpr auto SKIP_RENDER_HOOKS = false;

#include <gl/GL.h>

#include "hook/MinHook.h"

#include "libraries/imgui/imgui.h"
#include "libraries/imgui/imgui_impl_win32.h"
#include "libraries/imgui/imgui_impl_opengl2.h"
#include "libraries/imgui/imgui_internal.h"

#include "libraries/xorstr.h"
#include "version.h"
#include "wrapper/wrapper.h"
#include "utils/utils.h"
#include "mapper/mapper.h"
#include "utils/timer/timer.h"
