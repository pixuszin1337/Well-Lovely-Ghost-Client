#pragma once

#include "impl.h"
#include "../../module.h"

class c_instantstop : public c_module {
public:
	c_instantstop() : c_module(xorstr_("Instant Stop"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->local->get_object())
			return;

		if (!ctx->local->is_on_ground())
			return;

		bool w = GetAsyncKeyState('W') & 0x8000;
		bool a = GetAsyncKeyState('A') & 0x8000;
		bool s = GetAsyncKeyState('S') & 0x8000;
		bool d = GetAsyncKeyState('D') & 0x8000;

		if (!w && !a && !s && !d)
		{
			auto motion = ctx->local->get_motion_vector();
			ctx->local->get_motion_vector(sdk::vec3d{ 0.0, motion.y, 0.0 });
		}
	}
};
