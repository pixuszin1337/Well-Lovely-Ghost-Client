#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/input/input_hook.h"

class c_autoweapon : public c_module {
	bool m_was_clicking = false;

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	bool m_resolved = false;

	static float get_item_damage(JNIEnv* env, jobject item)
	{
		auto cls = env->GetObjectClass(item);

		auto fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("damageVsEntity") : BADLION_CLIENT ? xorstr_("c") : xorstr_("field_77827_a"),
			xorstr_("F"));
		if (!fid)
		{
			if (env->ExceptionCheck()) env->ExceptionClear();
			fid = env->GetFieldID(cls, xorstr_("attackDamage"), xorstr_("F"));
			if (!fid && env->ExceptionCheck()) env->ExceptionClear();
		}

		env->DeleteLocalRef(cls);
		if (!fid) return 0.0f;

		return env->GetFloatField(item, fid);
	}

public:
	c_autoweapon() : c_module(xorstr_("Auto Weapon"), e_category::combat) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object())
			return;

		bool clicking = input_hook::is_lmb_down();
		bool pressed_now = clicking && !m_was_clicking;
		m_was_clicking = clicking;

		if (!pressed_now)
			return;

		auto target = sdk::instance->get_entity_over(ctx->minecraft);
		if (!target)
			return;

		auto env = welllovely::instance->get_env();
		env->DeleteLocalRef(target);

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

		auto inv = env->GetObjectField(player, m_inv_fid);
		if (!inv) return;

		if (!m_main_fid || !m_cur_fid)
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

			if (!m_main_fid || !m_cur_fid) { env->DeleteLocalRef(inv); return; }
		}

		if (!m_getitem_mid)
		{
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

			if (!m_getitem_mid) { env->DeleteLocalRef(inv); return; }
		}

		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
		if (!arr) { env->DeleteLocalRef(inv); return; }

		jsize len = env->GetArrayLength(arr);
		if (len > 9) len = 9;

		int best_slot = -1;
		float best_dmg = 1.0f;

		for (jsize i = 0; i < len; i++)
		{
			auto stack = env->GetObjectArrayElement(arr, i);
			if (!stack)
			{
				if (env->ExceptionCheck()) env->ExceptionClear();
				continue;
			}

			float dmg = 0.0f;
			auto item = env->CallObjectMethod(stack, m_getitem_mid);
			if (item)
			{
				dmg = get_item_damage(env, item);
				env->DeleteLocalRef(item);
			}
			else if (env->ExceptionCheck())
			{
				env->ExceptionClear();
			}

			env->DeleteLocalRef(stack);

			if (dmg > best_dmg)
			{
				best_dmg = dmg;
				best_slot = (int)i;
			}
		}

		env->DeleteLocalRef(arr);

		if (best_slot >= 0)
		{
			jint cur = env->GetIntField(inv, m_cur_fid);
			if (cur != best_slot)
				env->SetIntField(inv, m_cur_fid, (jint)best_slot);
		}

		env->DeleteLocalRef(inv);
	}
};
