#pragma once

#include "cheat.h"
#include "config.h"
#include "../ui.h"

enum class e_category { combat, movement, player, render, utils, block, misc };

inline const char* category_name(e_category c)
{
	switch (c)
	{
	case e_category::combat:   return "Combat";
	case e_category::movement: return "Movement";
	case e_category::player:   return "Player";
	case e_category::render:   return "Render";
	case e_category::utils:    return "Utils";
	case e_category::block:    return "Block";
	default:                   return "Config";
	}
}

class c_module {
public:
	std::string name;
	e_category  category;
	bool        enabled = false;
	bool        was_enabled = false;
	int         keybind = 0;
	bool        show_toggle = true;

	c_module(std::string n, e_category c, int key = 0)
		: name(std::move(n)), category(c), keybind(key) {}

	virtual ~c_module() = default;

	virtual void on_tick(std::shared_ptr<c_context> ctx) {}

	virtual void on_render() {}

	virtual void on_draw(ImDrawList* draw) {}

	virtual void on_disable(std::shared_ptr<c_context> ctx) {}

	virtual void save_config(config_data& data) {}

	virtual void load_config(const config_data& data) {}
};

inline std::string vk_to_string(int vk)
{
	if (vk == 0)
		return "None";
	if (vk >= 'A' && vk <= 'Z')
		return std::string(1, static_cast<char>(vk));
	if (vk >= '0' && vk <= '9')
		return std::string(1, static_cast<char>(vk));
	if (vk >= VK_F1 && vk <= VK_F12)
		return "F" + std::to_string(vk - VK_F1 + 1);

	switch (vk)
	{
	case VK_SPACE:   return "Space";
	case VK_TAB:     return "Tab";
	case VK_SHIFT:   return "Shift";
	case VK_CONTROL: return "Ctrl";
	case VK_MENU:    return "Alt";
	case VK_RETURN:  return "Enter";
	case VK_ESCAPE:  return "Esc";
	case VK_UP:      return "Up";
	case VK_DOWN:    return "Down";
	case VK_LEFT:    return "Left";
	case VK_RIGHT:   return "Right";
	case VK_LBUTTON:  return "M1";
	case VK_RBUTTON:  return "M2";
	case VK_MBUTTON:  return "M3";
	case VK_XBUTTON1: return "M4";
	case VK_XBUTTON2: return "M5";
	}

	return "K" + std::to_string(vk);
}
