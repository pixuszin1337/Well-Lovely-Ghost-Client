#pragma once

#include "impl.h"
#include "../../module.h"
#include <chrono>

class c_safewalk : public c_module {
	unsigned m_last_keys = 0xffffffffu;
	float m_last_yaw = 1.0e9f;
	std::chrono::steady_clock::time_point m_last_change{};

	double m_written_x = 0.0;
	double m_written_z = 0.0;
	bool m_did_write = false;

	int m_safe_drop = 1;

public:
	c_safewalk() : c_module(xorstr_("Safe Walk"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;

	void on_render() override
	{
		ui::slider_int(xorstr_("Safe Drop"), &m_safe_drop, 0, 3, xorstr_("%d blocks"));
	}

	void save_config(config_data& d) override
	{
		d.set_int(xorstr_("sw_safe_drop"), m_safe_drop);
	}

	void load_config(const config_data& d) override
	{
		m_safe_drop = d.get_int(xorstr_("sw_safe_drop"), 1);
	}
};
