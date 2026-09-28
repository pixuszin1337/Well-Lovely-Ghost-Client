#pragma once

#include "impl.h"
#include "../../module.h"

class c_nullmove : public c_module {
public:
	c_nullmove() : c_module(xorstr_("Null Move"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->local->get_object())
			return;

		bool w = GetAsyncKeyState('W') & 0x8000;
		bool a = GetAsyncKeyState('A') & 0x8000;
		bool s = GetAsyncKeyState('S') & 0x8000;
		bool d = GetAsyncKeyState('D') & 0x8000;

		if (!w && !a && !s && !d)
		{
			ctx->local->set_move_forward(0.0f);
			ctx->local->set_move_strafe(0.0f);
		}
	}
};
