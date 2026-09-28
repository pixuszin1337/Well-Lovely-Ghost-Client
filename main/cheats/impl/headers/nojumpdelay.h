#pragma once

#include "impl.h"
#include "../../module.h"

class c_nojumpdelay : public c_module {
	jfieldID m_fid = nullptr;

public:
	c_nojumpdelay() : c_module(xorstr_("No Jump Delay"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->local->get_object())
			return;

		auto env = welllovely::instance->get_env();

		if (!m_fid)
		{
			auto cls = env->GetObjectClass(ctx->local->get_object());
			m_fid = env->GetFieldID(cls,
				CUSTOM_CLIENT ? xorstr_("jumpTicks") : BADLION_CLIENT ? xorstr_("be") : xorstr_("field_70773_bE"),
				xorstr_("I"));
			env->DeleteLocalRef(cls);

			if (!m_fid) { env->ExceptionClear(); return; }
		}

		env->SetIntField(ctx->local->get_object(), m_fid, 0);
	}
};
