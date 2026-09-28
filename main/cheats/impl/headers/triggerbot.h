#pragma once

#include "impl.h"
#include "../../module.h"

class c_triggerbot : public c_module {
public:
	int m_min_cps = 8;
	int m_max_cps = 12;

	c_triggerbot() : c_module(xorstr_("TriggerBot"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_render() override;
	void save_config(config_data& data) override;
	void load_config(const config_data& data) override;

private:
	std::chrono::steady_clock::time_point m_last_click{};
	int m_current_delay_ms = 100;

	void randomize_delay();
};
