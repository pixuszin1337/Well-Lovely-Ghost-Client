#pragma once

#include "impl.h"
#include "../../module.h"

class c_gamespeed : public c_module {
	float m_speed = 1.5f;

	jfieldID m_timer_fid = nullptr;
	jfieldID m_speed_fid = nullptr;
	bool m_resolved = false;

	void resolve(JNIEnv* env, jobject mc)
	{
		auto mc_cls = env->GetObjectClass(mc);
		m_timer_fid = env->GetFieldID(mc_cls,
			CUSTOM_CLIENT ? xorstr_("timer") : BADLION_CLIENT ? xorstr_("Y") : xorstr_("field_71428_T"),
			!BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/Timer;") : xorstr_("Lavl;"));
		env->DeleteLocalRef(mc_cls);

		if (!m_timer_fid) { env->ExceptionClear(); m_resolved = true; return; }

		auto timer = env->GetObjectField(mc, m_timer_fid);
		if (!timer) { m_resolved = true; return; }

		auto timer_cls = env->GetObjectClass(timer);
		m_speed_fid = env->GetFieldID(timer_cls,
			CUSTOM_CLIENT ? xorstr_("timerSpeed") : BADLION_CLIENT ? xorstr_("d") : xorstr_("field_74278_d"),
			xorstr_("F"));
		env->DeleteLocalRef(timer_cls);
		env->DeleteLocalRef(timer);

		if (!m_speed_fid) env->ExceptionClear();
		m_resolved = true;
	}

public:
	c_gamespeed() : c_module(xorstr_("Timer"), e_category::utils) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft) return;

		auto env = welllovely::instance->get_env();

		if (!m_resolved)
			resolve(env, ctx->minecraft);

		if (!m_timer_fid || !m_speed_fid) return;

		auto timer = env->GetObjectField(ctx->minecraft, m_timer_fid);
		if (!timer) return;

		env->SetFloatField(timer, m_speed_fid, m_speed);
		env->DeleteLocalRef(timer);
	}

	void on_disable(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft) return;

		auto env = welllovely::instance->get_env();

		if (m_timer_fid && m_speed_fid)
		{
			auto timer = env->GetObjectField(ctx->minecraft, m_timer_fid);
			if (timer)
			{
				env->SetFloatField(timer, m_speed_fid, 1.0f);
				env->DeleteLocalRef(timer);
			}
		}
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Speed"), &m_speed, 0.5f, 5.0f, xorstr_("%.1fx"));
	}

	void save_config(config_data& d) override { d.set_float(xorstr_("timer_speed"), m_speed); }
	void load_config(const config_data& d) override { m_speed = d.get_float(xorstr_("timer_speed"), 1.5f); }
};
