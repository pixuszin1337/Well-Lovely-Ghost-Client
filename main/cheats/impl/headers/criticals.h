#pragma once

#include "impl.h"
#include "../../module.h"

class c_criticals : public c_module {
public:
	c_criticals() : c_module(xorstr_("Criticals"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;

private:
	bool m_was_clicking = false;
};
