#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/auth/auth.h"

#include <random>

class c_reach : public c_module {
public:
	float m_min_reach = 3.0f;
	float m_max_reach = 3.5f;
	float m_rare_chance = 5.0f;
	float m_rare_dist = 3.0f;

	c_reach()
		: c_module(xorstr_("Reach"), e_category::combat, 0),
		  m_rng(std::random_device{}())
	{
		m_reach_base = auth::bits_to_float(0x40400000u);
	}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_render() override;

	void save_config(config_data& d) override {
		d.set_float("min_reach", m_min_reach);
		d.set_float("max_reach", m_max_reach);
		d.set_float("rare_chance", m_rare_chance);
		d.set_float("rare_dist", m_rare_dist);
	}
	void load_config(const config_data& d) override {
		m_min_reach = d.get_float("min_reach", 3.0f);
		m_max_reach = d.get_float("max_reach", 3.5f);
		m_rare_chance = d.get_float("rare_chance", 5.0f);
		m_rare_dist = d.get_float("rare_dist", 3.0f);
		m_reach_base = auth::bits_to_float(0x40400000u);
	}

private:
	c_timer      m_timer;
	std::mt19937 m_rng;
	float        m_reach_base;
};
