#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../visuals_config.h"
#include "../../../../sdk/render/render.h"

class c_tracers : public c_module {
public:
	c_tracers() : c_module(xorstr_("Tracers"), e_category::render) {}

	void on_draw(ImDrawList* draw) override;

private:
	sdk::c_render m_render;
};
