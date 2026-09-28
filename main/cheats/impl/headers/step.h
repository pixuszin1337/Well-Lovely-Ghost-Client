#pragma once

#include "impl.h"
#include "../../module.h"

class c_step : public c_module {
public:
	float m_height = 1.0f;

	c_step() : c_module(xorstr_("Step"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_disable(std::shared_ptr<c_context> ctx) override;
	void on_render() override;
	void save_config(config_data& data) override;
	void load_config(const config_data& data) override;
};
