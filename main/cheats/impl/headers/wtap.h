#pragma once

#include "impl.h"
#include "../../module.h"
#include <chrono>

class c_wtap : public c_module {
public:
	int m_interval = 10;

	c_wtap() : c_module(xorstr_("W-Tap"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_render() override;
	void save_config(config_data& data) override;
	void load_config(const config_data& data) override;

private:
	bool m_resetting = false;
	std::chrono::steady_clock::time_point m_last_reset{};
};
