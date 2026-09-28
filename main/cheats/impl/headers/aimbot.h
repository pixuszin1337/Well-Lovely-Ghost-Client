#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/auth/auth.h"

#include <chrono>

class c_aimbot : public c_module {
public:
	float m_speed = 10.0f;
	float m_speed_body = 2.0f;
	float m_fov = 30.0f;
	float m_max_distance = 0.0f;
	bool  m_only_attacking = false;
	bool  m_block_check = false;
	bool  m_ignore_invisible = true;

	c_aimbot()
		: c_module(xorstr_("Aim Assist"), e_category::combat, 0),
		  m_last(std::chrono::steady_clock::now())
	{
		m_aim_factor = auth::bits_to_float(0x3FD56042u);
	}

	void on_draw(ImDrawList* draw) override;
	void on_render() override;

	void save_config(config_data& d) override {
		d.set_float("speed", m_speed);
		d.set_float("speed_body", m_speed_body);
		d.set_float("fov", m_fov);
		d.set_float("max_distance", m_max_distance);
		d.set_bool("only_attacking", m_only_attacking);
		d.set_bool("block_check", m_block_check);
		d.set_bool("ignore_invisible", m_ignore_invisible);
	}
	void load_config(const config_data& d) override {
		m_speed = d.get_float("speed", 10.0f);
		m_speed_body = d.get_float("speed_body", 2.0f);
		m_fov = d.get_float("fov", 30.0f);
		m_max_distance = d.get_float("max_distance", 0.0f);
		m_only_attacking = d.get_bool("only_attacking", false);
		m_block_check = d.get_bool("block_check", false);
		m_ignore_invisible = d.get_bool("ignore_invisible", true);
		m_aim_factor = auth::bits_to_float(0x3FD56042u);
	}

private:
	std::chrono::steady_clock::time_point m_last;
	float m_aim_factor;
};
