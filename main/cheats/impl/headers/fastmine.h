#pragma once

#include "impl.h"
#include "../../module.h"

class c_fastmine : public c_module {
	float m_threshold = 0.8f;

	jfieldID m_pc_fid = nullptr;
	jfieldID m_hitdelay_fid = nullptr;
	jfieldID m_damage_fid = nullptr;
	bool m_resolved = false;

public:
	c_fastmine() : c_module(xorstr_("Fast Mine"), e_category::block) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft)
			return;

		auto env = welllovely::instance->get_env();

		if (!m_resolved)
		{
			m_resolved = true;

			auto mc_cls = env->GetObjectClass(ctx->minecraft);

			m_pc_fid = env->GetFieldID(mc_cls,
				CUSTOM_CLIENT ? xorstr_("playerController") : BADLION_CLIENT ? xorstr_("c") : xorstr_("field_71442_b"),
				!BADLION_CLIENT ? xorstr_("Lnet/minecraft/client/multiplayer/PlayerControllerMP;") : xorstr_("Lbje;"));
			env->DeleteLocalRef(mc_cls);

			if (!m_pc_fid) { env->ExceptionClear(); return; }

			auto pc = env->GetObjectField(ctx->minecraft, m_pc_fid);
			if (!pc) return;

			auto pc_cls = env->GetObjectClass(pc);

			m_hitdelay_fid = env->GetFieldID(pc_cls,
				CUSTOM_CLIENT ? xorstr_("blockHitDelay") : BADLION_CLIENT ? xorstr_("i") : xorstr_("field_78781_i"),
				xorstr_("I"));
			if (!m_hitdelay_fid && env->ExceptionCheck()) env->ExceptionClear();

			m_damage_fid = env->GetFieldID(pc_cls,
				CUSTOM_CLIENT ? xorstr_("curBlockDamageMP") : BADLION_CLIENT ? xorstr_("j") : xorstr_("field_78779_l"),
				xorstr_("F"));
			if (!m_damage_fid && env->ExceptionCheck()) env->ExceptionClear();

			env->DeleteLocalRef(pc_cls);
			env->DeleteLocalRef(pc);
		}

		if (!m_pc_fid || !m_hitdelay_fid || !m_damage_fid)
			return;

		auto pc = env->GetObjectField(ctx->minecraft, m_pc_fid);
		if (!pc) return;

		env->SetIntField(pc, m_hitdelay_fid, 0);

		float dmg = env->GetFloatField(pc, m_damage_fid);
		if (dmg > m_threshold)
			env->SetFloatField(pc, m_damage_fid, 1.0f);

		env->DeleteLocalRef(pc);
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Break Damage"), &m_threshold, 0.1f, 1.0f, xorstr_("%.2f"));
	}

	void save_config(config_data& d) override { d.set_float(xorstr_("fm_threshold"), m_threshold); }
	void load_config(const config_data& d) override { m_threshold = d.get_float(xorstr_("fm_threshold"), 0.8f); }
};
