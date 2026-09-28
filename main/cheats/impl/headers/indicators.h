#pragma once

#include "impl.h"
#include "../../module.h"

class c_indicators : public c_module {
	jfieldID m_inv_fid = nullptr;
	jfieldID m_armor_fid = nullptr;
	jfieldID m_dmg_fid = nullptr;
	jmethodID m_maxdmg_mid = nullptr;
	bool m_resolved = false;

public:
	c_indicators() : c_module(xorstr_("Indicators"), e_category::render) {}

	void on_draw(ImDrawList* draw) override
	{
		auto env = welllovely::instance->get_env();
		if (!env)
			return;

		auto mc = sdk::instance->get_minecraft();
		if (!mc)
			return;

		auto player = sdk::instance->get_player(mc);
		if (!player) { env->DeleteLocalRef(mc); return; }

		if (!m_resolved)
		{
			m_resolved = true;

			auto p_cls = env->GetObjectClass(player);
			m_inv_fid = env->GetFieldID(p_cls,
				CUSTOM_CLIENT ? xorstr_("inventory") : BADLION_CLIENT ? xorstr_("bo") : xorstr_("field_71071_by"),
				!BADLION_CLIENT ? xorstr_("Lnet/minecraft/entity/player/InventoryPlayer;") : xorstr_("Ladz;"));
			env->DeleteLocalRef(p_cls);

			if (!m_inv_fid) { env->ExceptionClear(); env->DeleteLocalRef(player); env->DeleteLocalRef(mc); return; }
		}

		auto inv = env->GetObjectField(player, m_inv_fid);
		if (!inv) { env->DeleteLocalRef(player); env->DeleteLocalRef(mc); return; }

		if (!m_armor_fid || !m_dmg_fid || !m_maxdmg_mid)
		{
			auto inv_cls = env->GetObjectClass(inv);
			m_armor_fid = env->GetFieldID(inv_cls,
				CUSTOM_CLIENT ? xorstr_("armorInventory") : BADLION_CLIENT ? xorstr_("c") : xorstr_("field_70460_c"),
				!BADLION_CLIENT ? xorstr_("[Lnet/minecraft/item/ItemStack;") : xorstr_("[Ladd;"));
			if (!m_armor_fid && env->ExceptionCheck()) env->ExceptionClear();
			env->DeleteLocalRef(inv_cls);

			auto stack_cls = welllovely::instance->find_class_quiet(
				!BADLION_CLIENT ? xorstr_("net.minecraft.item.ItemStack") : xorstr_("add"));
			if (stack_cls)
			{
				m_dmg_fid = env->GetFieldID(stack_cls,
					CUSTOM_CLIENT ? xorstr_("itemDamage") : BADLION_CLIENT ? xorstr_("i") : xorstr_("field_77991_d"),
					xorstr_("I"));
				if (!m_dmg_fid && env->ExceptionCheck()) env->ExceptionClear();

				m_maxdmg_mid = env->GetMethodID(stack_cls,
					CUSTOM_CLIENT ? xorstr_("getMaxDamage") : BADLION_CLIENT ? xorstr_("d") : xorstr_("func_77958_k"),
					xorstr_("()I"));
				if (!m_maxdmg_mid && env->ExceptionCheck()) env->ExceptionClear();

				env->DeleteLocalRef(stack_cls);
			}

			if (!m_armor_fid || !m_dmg_fid || !m_maxdmg_mid)
			{
				env->DeleteLocalRef(inv);
				env->DeleteLocalRef(player);
				env->DeleteLocalRef(mc);
				return;
			}
		}

		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_armor_fid));
		if (arr)
		{
			float fracs[4];
			int count = 0;

			jsize len = env->GetArrayLength(arr);
			if (len > 4) len = 4;

			for (jsize i = 0; i < len; i++)
			{
				auto stack = env->GetObjectArrayElement(arr, i);
				if (!stack)
				{
					if (env->ExceptionCheck()) env->ExceptionClear();
					fracs[count++] = -1.0f;
					continue;
				}

				jint maxd = env->CallIntMethod(stack, m_maxdmg_mid);
				if (env->ExceptionCheck()) { env->ExceptionClear(); maxd = 0; }

				jint dmg = env->GetIntField(stack, m_dmg_fid);

				float frac = -1.0f;
				if (maxd > 0)
					frac = 1.0f - (float)dmg / (float)maxd;
				else if (maxd == 0)
					frac = 1.0f;

				fracs[count++] = frac;
				env->DeleteLocalRef(stack);
			}

			auto& io = ImGui::GetIO();
			const float bw = 70.0f, bh = 7.0f, gap = 10.0f;
			const float x1 = io.DisplaySize.x - 20.0f;
			float y = io.DisplaySize.y * 0.5f - ((float)count * (bh + gap) - gap) * 0.5f;

			for (int i = 0; i < count; i++)
			{
				float frac = fracs[i];
				if (frac >= 0.0f)
				{
					if (frac > 1.0f) frac = 1.0f;

					draw->AddRectFilled(ImVec2(x1 - bw - 1.0f, y - 1.0f), ImVec2(x1 + 1.0f, y + bh + 1.0f), IM_COL32(0, 0, 0, 180));

					const ImU32 col = IM_COL32((int)((1.0f - frac) * 255.f), (int)(frac * 220.f + 35.f), 60, 255);
					draw->AddRectFilled(ImVec2(x1 - bw, y), ImVec2(x1 - bw + bw * frac, y + bh), col);
				}
				y += bh + gap;
			}

			env->DeleteLocalRef(arr);
		}

		env->DeleteLocalRef(inv);
		env->DeleteLocalRef(player);
		env->DeleteLocalRef(mc);
	}
};
