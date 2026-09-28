#include "../welllovely.h"

#include "../cheats/module_manager.h"

LRESULT imgui_wndproc(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!ImGui::GetCurrentContext())
		return 0;

	ImGuiIO& io = ImGui::GetIO();
	switch (msg) {
	case WM_KILLFOCUS:
		memset(io.KeysDown, 0, sizeof(io.KeysDown));
		io.KeyCtrl = io.KeyShift = io.KeyAlt = io.KeySuper = false;
		for (int i = 0; i < 5; ++i)
			io.MouseDown[i] = false;
		return 0;
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		return true;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return true;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		return true;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return true;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		return true;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return true;
	case WM_XBUTTONDOWN:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = true;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = true;
		return true;
	case WM_XBUTTONUP:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = false;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = false;
		return true;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		io.KeyCtrl  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		io.KeyShift = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
		io.KeyAlt   = (GetKeyState(VK_MENU)    & 0x8000) != 0;
		return true;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		io.KeyCtrl  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		io.KeyShift = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
		io.KeyAlt   = (GetKeyState(VK_MENU)    & 0x8000) != 0;
		return true;
	case WM_CHAR:
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}
	return 0;
}

LRESULT __stdcall welllovely::hooks::wndproc_hk(HWND hwnd, UINT usermsg, WPARAM wparam, LPARAM lparam)
{

	if (!welllovely::instance || !hooks::original_wndproc || !welllovely::instance->b_running
		|| !ImGui::GetCurrentContext())
		return DefWindowProcA(hwnd, usermsg, wparam, lparam);

	int pressed_vk = 0;

	if (WM_KEYDOWN == usermsg && !(lparam & (1 << 30)))
		pressed_vk = static_cast<int>(wparam);
	else if (WM_XBUTTONDOWN == usermsg)
		pressed_vk = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;

	if (pressed_vk)
	{

		if (modules::instance && modules::instance->is_binding())
		{
			modules::instance->feed_key(pressed_vk);
			return 1l;
		}

		if (pressed_vk == VK_UP)
			welllovely::instance->b_open = !welllovely::instance->b_open;

		else if (!welllovely::instance->b_open && modules::instance)
			modules::instance->feed_key(pressed_vk);
	}

	if (welllovely::instance->b_open && imgui_wndproc(hwnd, usermsg, wparam, lparam))
		return 1l;

	return CallWindowProcA(hooks::original_wndproc, hwnd, usermsg, wparam, lparam);
}
