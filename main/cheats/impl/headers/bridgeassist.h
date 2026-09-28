#pragma once

#include "impl.h"
#include "../../module.h"

class c_bridgeassist : public c_module {
	bool m_active = false;
	float m_min_pitch = 60.0f;

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getid_mid = nullptr;
	jclass m_item_class = nullptr;
	bool m_resolved = false;

	bool holding_block(JNIEnv* env, jobject player)
	{
		if (!m_inv_fid || !m_main_fid || !m_cur_fid || !m_getitem_mid || !m_getid_mid || !m_item_class)
			return false;

		auto inv = env->GetObjectField(player, m_inv_fid);
		if (!inv) return false;

		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
		env->DeleteLocalRef(inv);
		if (!arr) return false;

		jint cur = -1;
		{
			auto inv2 = env->GetObjectField(player, m_inv_fid);
			if (inv2)
			{
				cur = env->GetIntField(inv2, m_cur_fid);
				env->DeleteLocalRef(inv2);
			}
		}

		bool is_block = false;
		auto stack = env->GetObjectArrayElement(arr, cur);
		env->DeleteLocalRef(arr);

		if (stack)
		{
			auto item = env->CallObjectMethod(stack, m_getitem_mid);
			if (env->ExceptionCheck()) env->ExceptionClear();

			if (item)
			{
				jint id = env->CallStaticIntMethod(m_item_class, m_getid_mid, item);
				if (env->ExceptionCheck()) { env->ExceptionClear(); id = -1; }
				is_block = (id >= 1 && id <= 255);
				env->DeleteLocalRef(item);
			}
			env->DeleteLocalRef(stack);
		}
		else if (env->ExceptionCheck())
		{
			env->ExceptionClear();
		}

		return is_block;
	}

public:
	c_bridgeassist() : c_module(xorstr_("Bridge Assist"), e_category::block) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object())
			return;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();

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

		if (!m_main_fid || !m_cur_fid || !m_getitem_mid || !m_getid_mid || !m_item_class)
		{
			auto inv = env->GetObjectField(player, m_inv_fid);
			if (!inv) return;

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

			if (!m_main_fid || !m_cur_fid || !m_getitem_mid || !m_getid_mid || !m_item_class)
				return;
		}

		bool cond = ctx->local->is_on_ground()
			&& ctx->local->get_pitch() > m_min_pitch
			&& holding_block(env, player);

		if (cond && !m_active)
		{
			sdk::instance->set_use_key_pressed(ctx->minecraft, true);
			sdk::instance->right_click_mouse(ctx->minecraft);
			m_active = true;
		}
		else if (!cond && m_active)
		{
			if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
			{
				sdk::instance->right_click_release(ctx->minecraft);
				sdk::instance->set_use_key_pressed(ctx->minecraft, false);
			}
			m_active = false;
		}
	}

	void on_disable(std::shared_ptr<c_context> ctx) override
	{
		if (m_active && ctx->minecraft && !(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
		{
			sdk::instance->right_click_release(ctx->minecraft);
			sdk::instance->set_use_key_pressed(ctx->minecraft, false);
		}
		m_active = false;
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Min pitch"), &m_min_pitch, 40.0f, 85.0f, xorstr_("%.0f"));
	}

	void save_config(config_data& d) override { d.set_float(xorstr_("ba_pitch"), m_min_pitch); }
	void load_config(const config_data& d) override { m_min_pitch = d.get_float(xorstr_("ba_pitch"), 60.0f); }
};
