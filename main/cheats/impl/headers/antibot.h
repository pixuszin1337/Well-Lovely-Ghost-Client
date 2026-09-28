#pragma once

#include "impl.h"
#include "../../module.h"
#include <unordered_set>
#include <mutex>

class c_antibot : public c_module {
	static inline c_antibot* s_instance = nullptr;

	static inline jfieldID s_id_fid = nullptr;
	static inline jfieldID s_ticks_fid = nullptr;
	static inline jfieldID s_swing_fid = nullptr;
	static inline bool s_fields_resolved = false;

	static void resolve_fields(JNIEnv* env, jobject entity)
	{
		if (s_fields_resolved)
			return;
		s_fields_resolved = true;

		auto cls = env->GetObjectClass(entity);

		s_id_fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("entityId") : BADLION_CLIENT ? xorstr_("a") : xorstr_("field_145783_c"),
			xorstr_("I"));
		if (!s_id_fid && env->ExceptionCheck()) env->ExceptionClear();

		s_ticks_fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("ticksExisted") : BADLION_CLIENT ? xorstr_("w") : xorstr_("field_70173_aa"),
			xorstr_("I"));
		if (!s_ticks_fid && env->ExceptionCheck()) env->ExceptionClear();

		s_swing_fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("swingProgress") : BADLION_CLIENT ? xorstr_("aP") : xorstr_("field_70721_c"),
			xorstr_("F"));
		if (!s_swing_fid && env->ExceptionCheck()) env->ExceptionClear();

		env->DeleteLocalRef(cls);
	}

	static int get_entity_id(JNIEnv* env, jobject entity)
	{
		if (!s_id_fid)
			return -1;
		return (int)env->GetIntField(entity, s_id_fid);
	}

	static bool has_invalid_caps(JNIEnv* env, jobject player)
	{
		auto cls = env->GetObjectClass(player);
		auto caps_fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("capabilities") : BADLION_CLIENT ? xorstr_("bH") : xorstr_("field_71075_bZ"),
			!BADLION_CLIENT ? xorstr_("Lnet/minecraft/entity/player/PlayerCapabilities;") : xorstr_("Lwi;"));
		env->DeleteLocalRef(cls);

		if (!caps_fid)
		{
			env->ExceptionClear();
			return false;
		}

		auto caps = env->GetObjectField(player, caps_fid);
		if (!caps)
			return false;

		bool invalid = false;

		auto caps_cls = env->GetObjectClass(caps);

		const char* names[4] = { "isFlying", "allowFlying", "disableDamage", "isCreativeMode" };
		const char* srgs[4] = { "field_75100_b", "field_75101_c", "field_75098_b", "field_75099_a" };
		const char* notch[4] = { "b", "c", "a", "d" };

		for (int i = 0; i < 4 && !invalid; i++)
		{
			auto fid = env->GetFieldID(caps_cls,
				CUSTOM_CLIENT ? names[i] : BADLION_CLIENT ? notch[i] : srgs[i],
				xorstr_("Z"));
			if (!fid)
			{
				if (env->ExceptionCheck()) env->ExceptionClear();
				continue;
			}

			if (env->GetBooleanField(caps, fid) != JNI_FALSE)
				invalid = true;
		}

		env->DeleteLocalRef(caps_cls);
		env->DeleteLocalRef(caps);
		return invalid;
	}

public:
	bool m_check_id = true;
	bool m_check_derp = true;
	bool m_check_health = false;
	bool m_check_ticks = false;
	int m_min_ticks = 40;
	bool m_check_caps = true;
	bool m_check_ground = false;
	bool m_check_swing = false;

	std::unordered_set<int> m_ground_seen;
	std::unordered_set<int> m_swing_seen;
	std::mutex m_track_mutex;

	c_antibot() : c_module(xorstr_("Anti Bot"), e_category::utils)
	{
		s_instance = this;
	}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object() || !ctx->world)
			return;

		auto env = welllovely::instance->get_env();
		if (!env)
			return;

		static auto last_poll = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();
		if (now - last_poll < std::chrono::milliseconds(100))
			return;
		last_poll = now;

		if (!m_check_ground && !m_check_swing)
			return;

		std::lock_guard<std::mutex> lock(m_track_mutex);

		for (auto& player : ctx->world->get_players())
		{
			if (!player->get_object())
				continue;

			resolve_fields(env, player->get_object());
			if (!s_id_fid)
				continue;

			int id = get_entity_id(env, player->get_object());

			if (m_check_ground && player->is_on_ground())
				m_ground_seen.insert(id);

			if (m_check_swing && s_swing_fid)
			{
				float sw = env->GetFloatField(player->get_object(), s_swing_fid);
				if (sw > 0.01f && sw < 0.99f)
					m_swing_seen.insert(id);
			}
		}
	}

	static bool is_bot(c_player* player)
	{
		if (!s_instance || !s_instance->enabled || !player || !player->get_object())
			return false;
		auto inst = s_instance;
		auto env = welllovely::instance->get_env();
		if (!env)
			return false;

		resolve_fields(env, player->get_object());

		if (inst->m_check_id && s_id_fid)
		{
			int id = get_entity_id(env, player->get_object());
			if (id <= 0 || id >= 1000000000)
				return true;
		}

		if (inst->m_check_derp)
		{
			float pitch = player->get_pitch();
			if (pitch > 90.0f || pitch < -90.0f)
				return true;
		}

		if (inst->m_check_health)
		{
			float hp = player->get_health();
			if (hp != 1337.f && (hp > 20.0f || hp < 0.0f))
				return true;
		}

		if (inst->m_check_ticks && s_ticks_fid)
		{
			jint ticks = env->GetIntField(player->get_object(), s_ticks_fid);
			if (ticks < (jint)inst->m_min_ticks)
				return true;
		}

		if (inst->m_check_caps && has_invalid_caps(env, player->get_object()))
			return true;

		if (inst->m_check_ground || inst->m_check_swing)
		{
			int id = s_id_fid ? get_entity_id(env, player->get_object()) : -1;

			std::lock_guard<std::mutex> lock(inst->m_track_mutex);

			if (inst->m_check_ground && inst->m_ground_seen.count(id) == 0)
				return true;

			if (inst->m_check_swing && inst->m_swing_seen.count(id) == 0)
				return true;
		}

		return false;
	}

public:
	void on_render() override
	{
		ui::toggle(xorstr_("Entity ID"), &m_check_id);
		ui::toggle(xorstr_("Derp pitch"), &m_check_derp);
		ui::toggle(xorstr_("Health"), &m_check_health);
		ui::toggle(xorstr_("Living time"), &m_check_ticks);
		ui::toggle(xorstr_("Capabilities"), &m_check_caps);
		ui::toggle(xorstr_("Never on ground"), &m_check_ground);
		ui::toggle(xorstr_("Never swung"), &m_check_swing);
	}

	void save_config(config_data& d) override
	{
		d.set_int(xorstr_("ab_id"), m_check_id);
		d.set_int(xorstr_("ab_derp"), m_check_derp);
		d.set_int(xorstr_("ab_health"), m_check_health);
		d.set_int(xorstr_("ab_ticks"), m_check_ticks);
		d.set_int(xorstr_("ab_caps"), m_check_caps);
		d.set_int(xorstr_("ab_ground"), m_check_ground);
		d.set_int(xorstr_("ab_swing"), m_check_swing);
	}

	void load_config(const config_data& d) override
	{
		m_check_id = d.get_int(xorstr_("ab_id"), 1);
		m_check_derp = d.get_int(xorstr_("ab_derp"), 1);
		m_check_health = d.get_int(xorstr_("ab_health"), 0);
		m_check_ticks = d.get_int(xorstr_("ab_ticks"), 0);
		m_check_caps = d.get_int(xorstr_("ab_caps"), 1);
		m_check_ground = d.get_int(xorstr_("ab_ground"), 0);
		m_check_swing = d.get_int(xorstr_("ab_swing"), 0);
	}
};
