#pragma once

#include "impl.h"
#include "../../module.h"
#include <random>

class c_velocity : public c_module {
public:
	float m_horizontal = 100.0f;
	float m_vertical = 100.0f;
	bool  m_jump_reset = false;
	float m_jump_reset_chance = 100.0f;

	c_velocity() : c_module(xorstr_("Velocity"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_render() override;

	void save_config(config_data& d) override {
		d.set_float("horizontal", m_horizontal);
		d.set_float("vertical", m_vertical);
		d.set_bool("jump_reset", m_jump_reset);
		d.set_float("jump_reset_chance", m_jump_reset_chance);
	}
	void load_config(const config_data& d) override {
		m_horizontal = d.get_float("horizontal", 100.0f);
		m_vertical = d.get_float("vertical", 100.0f);
		m_jump_reset = d.get_bool("jump_reset", false);
		m_jump_reset_chance = d.get_float("jump_reset_chance", 100.0f);
	}

private:
	float m_prev_hurt = 0.0f;
	bool  m_space_down = false;
	std::mt19937 m_rng{ std::random_device{}() };
};
