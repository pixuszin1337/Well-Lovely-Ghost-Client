#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../visuals_config.h"
#include "../../../../sdk/render/render.h"

class c_esp : public c_module {
public:
	c_esp() : c_module(xorstr_("ESP"), e_category::render) {}

	void on_draw(ImDrawList* draw) override;
	void on_render() override;

	void save_config(config_data& d) override {
		d.set_bool("health", m_health);
	}
	void load_config(const config_data& d) override {
		m_health = d.get_bool("health", true);
	}

	bool  m_health = true;

private:
	sdk::c_render m_render;
};
