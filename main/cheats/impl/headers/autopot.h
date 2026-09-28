#pragma once

#include "impl.h"
#include "../../module.h"
#include <chrono>

class c_autopot : public c_module {
	enum class e_state { idle, aim, thrown };

	float m_health = 12.0f;
	e_state m_state = e_state::idle;
	float m_saved_pitch = 0.0f;
	int m_return_slot = -1;
	int m_pot_slot = -1;
	std::chrono::steady_clock::time_point m_phase{};
	std::chrono::steady_clock::time_point m_last_check{};

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getmeta_mid = nullptr;
	jmethodID m_getid_mid = nullptr;
	jclass m_item_class = nullptr;
	bool m_resolved = false;

	static bool is_splash_heal(int meta)
	{
		static const int splash_heal[] = { 16389, 16421, 16385, 16449, 16417 };
		for (int m : splash_heal)
			if (m == meta)
				return true;
		return false;
	}

	void abort(std::shared_ptr<c_context> ctx)
	{
		if (!ctx->local->get_object() || !ctx->minecraft)
		{
			m_state = e_state::idle;
			return;
		}

		if (m_saved_pitch != 0.0f)
		{
			ctx->local->set_pitch(m_saved_pitch);
			m_saved_pitch = 0.0f;
		}

		if (m_pot_slot >= 0 && m_return_slot >= 0 && m_inv_fid && m_cur_fid)
		{
			auto env = welllovely::instance->get_env();
			auto inv = env->GetObjectField(ctx->local->get_object(), m_inv_fid);
			if (inv)
			{
				jint cur = env->GetIntField(inv, m_cur_fid);
				if (cur == (jint)m_pot_slot)
					env->SetIntField(inv, m_cur_fid, (jint)m_return_slot);
				env->DeleteLocalRef(inv);
			}
		}

		if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
			sdk::instance->right_click_release(ctx->minecraft);

		m_state = e_state::idle;
		m_pot_slot = -1;
		m_return_slot = -1;
	}

public:
	c_autopot() : c_module(xorstr_("Auto Pot"), e_category::utils) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object())
			return;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();
		auto now = std::chrono::steady_clock::now();

		switch (m_state)
		{
		case e_state::idle:
		{
			if (now - m_last_check < std::chrono::milliseconds(150))
				return;
			m_last_check = now;

			float hp = ctx->local->get_health();
			if (hp == 1337.f || hp > m_health)
				return;

			if (!m_resolved)
			{
				m_resolved = true;

				auto p_cls = env->GetObjectClass(player);
				m_inv_fid = env->GetFieldID(p_cls,
					CUSTOM_CLIENT ? xorstr_("inventory") : BADLION_CLIENT ? xorstr_("bo") : xorstr_("field_71071_by"),
					!BADLION_CLIENT ? xorstr_("Lnet/minecraft/entity/player/InventoryPlayer;") : xorstr_("Ladz;"));
				env->DeleteLocalRef(p_cls);
				if (!m_inv_fid) { env->ExceptionClear(); return; }
			}

			auto inv = env->GetObjectField(player, m_inv_fid);
			if (!inv) return;

			if (!m_main_fid || !m_cur_fid || !m_getitem_mid || !m_getmeta_mid || !m_getid_mid || !m_item_class)
			{
				auto inv_cls = env->GetObjectClass(inv);
				m_main_fid = env->GetFieldID(inv_cls,
					CUSTOM_CLIENT ? xorstr_("mainInventory") : BADLION_CLIENT ? xorstr_("a") : xorstr_("field_70462_a"),
					!BADLION_CLIENT ? xorstr_("[Lnet/minecraft/item/ItemStack;") : xorstr_("[Ladd;"));
				if (!m_main_fid && env->ExceptionCheck()) env->ExceptionClear();

				m_cur_fid = env->GetFieldID(inv_cls,
					CUSTOM_CLIENT ? xorstr_("currentItem") : BADLION_CLIENT ? xorstr_("b") : xorstr_("field_70461_c"),
					xorstr_("I"));
				if (!m_cur_fid && env->ExceptionCheck()) env->ExceptionClear();

				env->DeleteLocalRef(inv_cls);

				auto stack_cls = welllovely::instance->find_class_quiet(
					!BADLION_CLIENT ? xorstr_("net.minecraft.item.ItemStack") : xorstr_("add"));
				if (stack_cls)
				{
					m_getitem_mid = env->GetMethodID(stack_cls,
						CUSTOM_CLIENT ? xorstr_("getItem") : BADLION_CLIENT ? xorstr_("u") : xorstr_("func_77973_b"),
						xorstr_("()Lnet/minecraft/item/Item;"));
					if (!m_getitem_mid && env->ExceptionCheck()) env->ExceptionClear();

					m_getmeta_mid = env->GetMethodID(stack_cls,
						CUSTOM_CLIENT ? xorstr_("getMetadata") : BADLION_CLIENT ? xorstr_("i") : xorstr_("func_77960_j"),
						xorstr_("()I"));
					if (!m_getmeta_mid && env->ExceptionCheck()) env->ExceptionClear();

					env->DeleteLocalRef(stack_cls);
				}

				auto item_cls = welllovely::instance->find_class_quiet(
					!BADLION_CLIENT ? xorstr_("net.minecraft.item.Item") : xorstr_("afp"));
				if (item_cls)
				{
					m_getid_mid = env->GetStaticMethodID(item_cls,
						CUSTOM_CLIENT ? xorstr_("getIdFromItem") : BADLION_CLIENT ? xorstr_("b") : xorstr_("func_150891_b"),
						xorstr_("(Lnet/minecraft/item/Item;)I"));
					if (!m_getid_mid && env->ExceptionCheck()) env->ExceptionClear();

					if (m_getid_mid)
						m_item_class = (jclass)env->NewGlobalRef(item_cls);

					env->DeleteLocalRef(item_cls);
				}

				if (!m_main_fid || !m_cur_fid || !m_getitem_mid || !m_getmeta_mid || !m_getid_mid || !m_item_class)
				{
					env->DeleteLocalRef(inv);
					return;
				}
			}

			auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
			if (!arr) { env->DeleteLocalRef(inv); return; }

			jsize len = env->GetArrayLength(arr);
			if (len > 9) len = 9;

			int pot_slot = -1;

			for (jsize i = 0; i < len && pot_slot < 0; i++)
			{
				auto stack = env->GetObjectArrayElement(arr, i);
				if (!stack)
				{
					if (env->ExceptionCheck()) env->ExceptionClear();
					continue;
				}

				auto item = env->CallObjectMethod(stack, m_getitem_mid);
				if (env->ExceptionCheck()) env->ExceptionClear();

				if (item)
				{
					jint id = env->CallStaticIntMethod(m_item_class, m_getid_mid, item);
					if (env->ExceptionCheck()) { env->ExceptionClear(); id = -1; }

					if (id == 373)
					{
						jint meta = env->CallIntMethod(stack, m_getmeta_mid);
						if (env->ExceptionCheck()) { env->ExceptionClear(); meta = -1; }

						if (is_splash_heal((int)meta))
							pot_slot = (int)i;
					}

					env->DeleteLocalRef(item);
				}

				env->DeleteLocalRef(stack);
			}

			env->DeleteLocalRef(arr);

			if (pot_slot < 0)
			{
				env->DeleteLocalRef(inv);
				return;
			}

			jint cur = env->GetIntField(inv, m_cur_fid);
			m_return_slot = (int)cur;
			m_pot_slot = pot_slot;

			if (cur != (jint)pot_slot)
				env->SetIntField(inv, m_cur_fid, (jint)pot_slot);

			env->DeleteLocalRef(inv);

			m_saved_pitch = ctx->local->get_pitch();
			ctx->local->set_pitch(85.0f);

			m_state = e_state::aim;
			m_phase = now;
			break;
		}

		case e_state::aim:
			if (now - m_phase < std::chrono::milliseconds(120))
				break;

			sdk::instance->right_click_mouse(ctx->minecraft);
			m_state = e_state::thrown;
			m_phase = now;
			break;

		case e_state::thrown:
			if (now - m_phase < std::chrono::milliseconds(120))
				break;

			abort(ctx);
			break;
		}
	}

	void on_disable(std::shared_ptr<c_context> ctx) override
	{
		if (m_state != e_state::idle)
			abort(ctx);
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Health"), &m_health, 4.0f, 18.0f, xorstr_("%.0f"));
	}

	void save_config(config_data& d) override { d.set_float(xorstr_("ap_health"), m_health); }
	void load_config(const config_data& d) override { m_health = d.get_float(xorstr_("ap_health"), 12.0f); }
};
