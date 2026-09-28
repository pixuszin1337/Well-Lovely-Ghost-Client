#pragma once

#include "impl.h"
#include "../../module.h"
#include <chrono>

class c_antidebuff : public c_module {
	jfieldID m_map_fid = nullptr;
	jclass m_integer_cls = nullptr;
	jmethodID m_valueof_mid = nullptr;
	jmethodID m_remove_mid = nullptr;
	bool m_resolved = false;
	std::chrono::steady_clock::time_point m_last_run{};

public:
	c_antidebuff() : c_module(xorstr_("Anti Debuff"), e_category::render) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->local->get_object())
			return;

		auto now = std::chrono::steady_clock::now();
		if (now - m_last_run < std::chrono::milliseconds(150))
			return;
		m_last_run = now;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();

		if (!m_resolved)
		{
			m_resolved = true;

			auto cls = env->GetObjectClass(player);
			m_map_fid = env->GetFieldID(cls,
				CUSTOM_CLIENT ? xorstr_("activePotionsMap") : BADLION_CLIENT ? xorstr_("bk") : xorstr_("field_70748_bN"),
				xorstr_("Ljava/util/HashMap;"));
			env->DeleteLocalRef(cls);

			if (!m_map_fid) { env->ExceptionClear(); return; }

			auto local_int = env->FindClass(xorstr_("java/lang/Integer"));
			if (local_int)
			{
				m_valueof_mid = env->GetStaticMethodID(local_int, xorstr_("valueOf"), xorstr_("(I)Ljava/lang/Integer;"));
				m_integer_cls = (jclass)env->NewGlobalRef(local_int);
				env->DeleteLocalRef(local_int);
			}

			if (!m_valueof_mid || !m_integer_cls) { env->ExceptionClear(); return; }
		}

		if (!m_map_fid || !m_integer_cls || !m_valueof_mid)
			return;

		auto map = env->GetObjectField(player, m_map_fid);
		if (!map)
			return;

		if (!m_remove_mid)
		{
			auto map_cls = env->GetObjectClass(map);
			m_remove_mid = env->GetMethodID(map_cls, xorstr_("remove"), xorstr_("(Ljava/lang/Object;)Ljava/lang/Object;"));
			env->DeleteLocalRef(map_cls);

			if (!m_remove_mid) { env->ExceptionClear(); env->DeleteLocalRef(map); return; }
		}

		static const int bad_ids[] = { 2, 4, 9, 15, 17, 18, 19, 20 };

		for (int id : bad_ids)
		{
			auto key = env->CallStaticObjectMethod(m_integer_cls, m_valueof_mid, (jint)id);
			if (!key) { env->ExceptionClear(); continue; }

			auto removed = env->CallObjectMethod(map, m_remove_mid, key);
			if (env->ExceptionCheck()) env->ExceptionClear();
			if (removed) env->DeleteLocalRef(removed);
			env->DeleteLocalRef(key);
		}

		env->DeleteLocalRef(map);
	}
};
