#pragma once

#include "impl.h"
#include "../../module.h"

class c_nohitdelay : public c_module {
public:
	c_nohitdelay() : c_module(xorstr_("No Hit Delay"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft)
			return;

		auto env = welllovely::instance->get_env();
		auto cls = env->GetObjectClass(ctx->minecraft);

		auto fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("leftClickCounter") : BADLION_CLIENT ? xorstr_("ag") : xorstr_("field_71429_W"),
			xorstr_("I"));

		env->DeleteLocalRef(cls);

		if (!fid) { env->ExceptionClear(); return; }

		env->SetIntField(ctx->minecraft, fid, 0);
	}
};
