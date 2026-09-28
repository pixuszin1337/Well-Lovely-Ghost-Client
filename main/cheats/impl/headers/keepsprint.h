#pragma once

#include "impl.h"
#include "../../module.h"

class c_keepsprint : public c_module {
public:
	c_keepsprint() : c_module(xorstr_("Keep Sprint"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->local->get_object())
			return;

		if (GetAsyncKeyState('W') & 0x8000)
			ctx->local->set_sprinting(true);
	}
};
