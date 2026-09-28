#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/utils/wl_log.h"
#include <chrono>

class c_refill : public c_module {
	int m_mode = 0;
	int m_min_items = 2;
	int m_delay_ms = 200;

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getmeta_mid = nullptr;
	jmethodID m_getid_mid = nullptr;
	jclass m_item_class = nullptr;
	bool m_resolved = false;

	std::chrono::steady_clock::time_point m_last_click{};

	static bool is_heal_potion_meta(int meta)
	{
		static const int heal_meta[] = {
			8193, 8257, 8225,
			16385, 16449, 16417,
			8197, 8229,
			16389, 16421
		};

		for (int m : heal_meta)
			if (m == meta)
				return true;
		return false;
	}

public:
	c_refill() : c_module(xorstr_("Refill"), e_category::utils) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object())
			return;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();

		auto screen = sdk::instance->get_current_screen(ctx->minecraft);
		if (screen)
		{
			env->DeleteLocalRef(screen);
			wl::log_throttled("refill_screen", 2000, "REFILL: tela aberta (skip)");
			return;
		}

		auto now = std::chrono::steady_clock::now();
		if (now - m_last_click < std::chrono::milliseconds(m_delay_ms))
			return;
		m_last_click = now;

		if (!m_resolved)
		{
			m_resolved = true;

			auto p_cls = env->GetObjectClass(player);
			m_inv_fid = env->GetFieldID(p_cls,
				CUSTOM_CLIENT ? xorstr_("inventory") : BADLION_CLIENT ? xorstr_("bo") : xorstr_("field_71071_by"),
				!BADLION_CLIENT ? xorstr_("Lnet/minecraft/entity/player/InventoryPlayer;") : xorstr_("Ladz;"));
			env->DeleteLocalRef(p_cls);

			if (!m_inv_fid) { env->ExceptionClear(); return; }

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

			if (!m_getitem_mid || !m_getid_mid || !m_item_class || (m_mode == 1 && !m_getmeta_mid))
				return;
		}

		auto inv = env->GetObjectField(player, m_inv_fid);
		if (!inv) return;

		if (!m_main_fid)
		{
			auto inv_cls = env->GetObjectClass(inv);
			m_main_fid = env->GetFieldID(inv_cls,
				CUSTOM_CLIENT ? xorstr_("mainInventory") : BADLION_CLIENT ? xorstr_("a") : xorstr_("field_70462_a"),
				!BADLION_CLIENT ? xorstr_("[Lnet/minecraft/item/ItemStack;") : xorstr_("[Ladd;"));
			env->DeleteLocalRef(inv_cls);

			if (!m_main_fid) { env->ExceptionClear(); env->DeleteLocalRef(inv); return; }
		}

		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
		if (!arr) { env->DeleteLocalRef(inv); return; }

		jsize len = env->GetArrayLength(arr);
		if (len > 36) len = 36;

		const int target_id = (m_mode == 0) ? 282 : 373;

		int hotbar_count = 0;
		int storage_slot = -1;

		for (jsize i = 0; i < len; i++)
		{
			auto stack = env->GetObjectArrayElement(arr, i);
			if (!stack)
			{
				if (env->ExceptionCheck()) env->ExceptionClear();
				continue;
			}

			bool match = false;

			auto item = env->CallObjectMethod(stack, m_getitem_mid);
			if (env->ExceptionCheck()) env->ExceptionClear();

			if (item)
			{
				jint id = env->CallStaticIntMethod(m_item_class, m_getid_mid, item);
				if (env->ExceptionCheck()) { env->ExceptionClear(); id = -1; }

				if (id == target_id)
				{
					if (m_mode == 0)
					{
						match = true;
					}
					else if (m_getmeta_mid)
					{
						jint meta = env->CallIntMethod(stack, m_getmeta_mid);
						if (env->ExceptionCheck()) { env->ExceptionClear(); meta = -1; }
						match = is_heal_potion_meta((int)meta);
					}
				}

				env->DeleteLocalRef(item);
			}

			env->DeleteLocalRef(stack);

			if (match)
			{
				if (i < 9)
					hotbar_count++;
				else if (storage_slot < 0)
					storage_slot = (int)i;
			}
		}

		env->DeleteLocalRef(arr);
		env->DeleteLocalRef(inv);

		wl::log_throttled("refill_scan", 1000, "REFILL: mode=%d hotbar=%d min=%d storage=%d",
			m_mode, hotbar_count, m_min_items, storage_slot);

		if (hotbar_count >= m_min_items || storage_slot < 0)
			return;

		wl::log("REFILL: clicando slot=%d (shift)", storage_slot);
		sdk::instance->window_click(ctx->minecraft, storage_slot, 0, 1);
	}

	void on_render() override
	{
		ImGui::TextUnformatted(xorstr_("Mode"));
		ImGui::SameLine();
		ImGui::RadioButton(xorstr_("Soup"), &m_mode, 0);
		ImGui::SameLine();
		ImGui::RadioButton(xorstr_("Potion"), &m_mode, 1);

		ui::slider_int(xorstr_("Min in hotbar"), &m_min_items, 1, 8, xorstr_("%d"));
		ui::slider_int(xorstr_("Delay"), &m_delay_ms, 50, 500, xorstr_("%d ms"));
	}

	void save_config(config_data& d) override
	{
		d.set_int(xorstr_("rf_mode"), m_mode);
		d.set_int(xorstr_("rf_min"), m_min_items);
		d.set_int(xorstr_("rf_delay"), m_delay_ms);
	}

	void load_config(const config_data& d) override
	{
		m_mode = d.get_int(xorstr_("rf_mode"), 0);
		m_min_items = d.get_int(xorstr_("rf_min"), 2);
		m_delay_ms = d.get_int(xorstr_("rf_delay"), 200);
	}
};
