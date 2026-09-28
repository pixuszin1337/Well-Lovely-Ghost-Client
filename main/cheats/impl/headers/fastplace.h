#pragma once

#include "impl.h"
#include "../../module.h"

class c_fastplace : public c_module {
public:
	int m_delay_ticks = 0;

	c_fastplace() : c_module(xorstr_("Fast Place"), e_category::block) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_render() override;
	void save_config(config_data& data) override;
	void load_config(const config_data& data) override;
};
