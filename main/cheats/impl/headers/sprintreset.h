#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/input/input_hook.h"

class c_sprintreset : public c_module {
	bool m_was_clicking = false;

public:
	c_sprintreset() : c_module(xorstr_("Sprint Reset"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object())
			return;

		bool clicking = input_hook::is_lmb_down();
		bool pressed_now = clicking && !m_was_clicking;
		m_was_clicking = clicking;

		if (!pressed_now)
			return;

		if (!ctx->local->is_sprinting())
			return;

		auto target = sdk::instance->get_entity_over(ctx->minecraft);
		if (!target)
			return;

		auto env = welllovely::instance->get_env();
		env->DeleteLocalRef(target);

		ctx->local->set_sprinting(false);
	}
};
