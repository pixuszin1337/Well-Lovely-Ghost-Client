#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/render/render.h"

class c_nametags : public c_module {
public:
	c_nametags() : c_module(xorstr_("NameTags"), e_category::render) {}

	void on_draw(ImDrawList* draw) override;

private:
	sdk::c_render m_render;
};
