#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/auth/auth.h"

#include <random>

class c_autoclicker : public c_module {
public:
	int   m_mode = 1;
	float m_cps = 12.0f;
	float m_min_cps = 8.0f;
	float m_max_cps = 14.0f;
	bool  m_block_check = false;

	c_autoclicker()
		: c_module(xorstr_("AutoClicker"), e_category::combat, 0),
		  m_rng(std::random_device{}())
	{
		m_ms_base = auth::bits_to_float(0x447A0000u);
	}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_render() override;

	void save_config(config_data& d) override {
		d.set_int("mode", m_mode);
		d.set_float("cps", m_cps);
		d.set_float("min_cps", m_min_cps);
		d.set_float("max_cps", m_max_cps);
		d.set_bool("block_check", m_block_check);
	}
	void load_config(const config_data& d) override {
		m_mode = d.get_int("mode", 1);
		m_cps = d.get_float("cps", 12.0f);
		m_min_cps = d.get_float("min_cps", 8.0f);
		m_max_cps = d.get_float("max_cps", 14.0f);
		m_block_check = d.get_bool("block_check", false);
		m_ms_base = auth::bits_to_float(0x447A0000u);
	}

private:
	c_timer      m_timer;
	c_timer      m_up_timer;
	std::mt19937 m_rng;
	long         m_next_delay = 0;
	long         m_up_delay = 0;
	bool         m_click_pending = false;
	float        m_ms_base;
};
