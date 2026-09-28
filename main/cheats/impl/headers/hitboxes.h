#pragma once

#include "impl.h"
#include "../../module.h"

class c_hitboxes : public c_module {
public:
	float m_size = 0.2f;

	c_hitboxes() : c_module(xorstr_("HitBoxes"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_render() override;

	void save_config(config_data& d) override {
		d.set_float("size", m_size);
	}
	void load_config(const config_data& d) override {
		m_size = d.get_float("size", 0.2f);
	}

private:
	c_timer m_timer;
};
