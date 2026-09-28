#pragma once

#include "impl.h"
#include "../../module.h"

class c_fullbright : public c_module {
public:
	c_fullbright() : c_module(xorstr_("Fullbright"), e_category::render) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
	void on_disable(std::shared_ptr<c_context> ctx) override;

private:
	float m_original = 1.0f;
	bool  m_saved = false;
};
