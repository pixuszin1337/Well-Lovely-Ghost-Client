#pragma once

#include "impl.h"
#include "../../module.h"

class c_nousedelay : public c_module {
public:
	c_nousedelay() : c_module(xorstr_("No Use Delay"), e_category::block) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft)
			return;

		auto env = welllovely::instance->get_env();
		auto cls = env->GetObjectClass(ctx->minecraft);

		auto fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("rightClickDelayTimer") : BADLION_CLIENT ? xorstr_("ap") : xorstr_("field_71467_ac"),
			xorstr_("I"));

		env->DeleteLocalRef(cls);

		if (!fid) { env->ExceptionClear(); return; }

		env->SetIntField(ctx->minecraft, fid, 0);
	}
};
