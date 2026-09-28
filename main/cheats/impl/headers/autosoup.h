#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/utils/wl_log.h"
#include <chrono>

class c_autosoup : public c_module {
	float m_health = 12.0f;
	bool m_drop_bowl = true;

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getid_mid = nullptr;
	jclass m_item_class = nullptr;
	bool m_resolved = false;

	bool m_eating = false;
	int m_soup_slot = -1;
	int m_return_slot = -1;
	std::chrono::steady_clock::time_point m_eat_start{};

	std::chrono::steady_clock::time_point m_last_check{};

	void finish(std::shared_ptr<c_context> ctx)
	{
		if (!ctx->minecraft)
			return;

		auto env = welllovely::instance->get_env();

		bool user_rmb = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
		if (!user_rmb)
		{
			sdk::instance->right_click_release(ctx->minecraft);
			sdk::instance->set_use_key_pressed(ctx->minecraft, false);
		}

		if (m_eating && ctx->local->get_object() && m_inv_fid && m_cur_fid)
		{
			auto inv = env->GetObjectField(ctx->local->get_object(), m_inv_fid);
			if (inv)
			{
				jint cur = env->GetIntField(inv, m_cur_fid);

				if (m_drop_bowl && cur == (jint)m_soup_slot && m_main_fid && m_getitem_mid && m_getid_mid && m_item_class)
				{
					auto screen = sdk::instance->get_current_screen(ctx->minecraft);
					bool gui_open = screen != nullptr;
					if (screen) env->DeleteLocalRef(screen);

					if (!gui_open)
					{
						auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
						if (arr)
						{
							auto stack = env->GetObjectArrayElement(arr, cur);
							if (stack)
							{
								jint id = -1;
								auto item = env->CallObjectMethod(stack, m_getitem_mid);
								if (env->ExceptionCheck()) env->ExceptionClear();

								if (item)
								{
									id = env->CallStaticIntMethod(m_item_class, m_getid_mid, item);
									if (env->ExceptionCheck()) { env->ExceptionClear(); id = -1; }
									env->DeleteLocalRef(item);
								}

								env->DeleteLocalRef(stack);

								wl::log("SOUP: fim, item no slot id=%d", id);
								if (id == 281)
								{
									wl::log("SOUP: dropando tigela (cont slot %d)", 36 + m_soup_slot);
									sdk::instance->window_click(ctx->minecraft, 36 + m_soup_slot, 1, 4);
								}
							}
							else if (env->ExceptionCheck())
							{
								env->ExceptionClear();
							}

							env->DeleteLocalRef(arr);
						}
					}
				}

				if (cur == (jint)m_soup_slot && m_return_slot >= 0)
					env->SetIntField(inv, m_cur_fid, (jint)m_return_slot);
				env->DeleteLocalRef(inv);
			}
		}

		m_eating = false;
		m_soup_slot = -1;
		m_return_slot = -1;
	}

public:
	c_autosoup() : c_module(xorstr_("Auto Soup"), e_category::utils) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object())
			return;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();

		auto now = std::chrono::steady_clock::now();

		if (!m_eating)
		{
			if (now - m_last_check < std::chrono::milliseconds(100))
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

			if (!m_main_fid || !m_cur_fid || !m_getitem_mid || !m_getid_mid)
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
				{
					env->DeleteLocalRef(inv);
					return;
				}
			}

			auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
			if (!arr) { env->DeleteLocalRef(inv); return; }

			jsize len = env->GetArrayLength(arr);
			if (len > 9) len = 9;

			int soup_slot = -1;

			for (jsize i = 0; i < len && soup_slot < 0; i++)
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

					if (id == 282)
						soup_slot = (int)i;

					env->DeleteLocalRef(item);
				}

				env->DeleteLocalRef(stack);
			}

			env->DeleteLocalRef(arr);

			if (soup_slot < 0)
			{
				env->DeleteLocalRef(inv);
				return;
			}

			jint cur = env->GetIntField(inv, m_cur_fid);
			m_return_slot = (int)cur;
			m_soup_slot = soup_slot;

			if (cur != (jint)soup_slot)
				env->SetIntField(inv, m_cur_fid, (jint)soup_slot);

			env->DeleteLocalRef(inv);

			sdk::instance->set_use_key_pressed(ctx->minecraft, true);
			sdk::instance->right_click_mouse(ctx->minecraft);
			wl::log("SOUP: comecando slot=%d (return=%d)", soup_slot, m_return_slot);
			m_eating = true;
			m_eat_start = now;
			return;
		}

		sdk::instance->set_use_key_pressed(ctx->minecraft, true);

		bool done = false;

		float hp = ctx->local->get_health();
		if (hp != 1337.f && hp > m_health)
			done = true;

		if (now - m_eat_start > std::chrono::milliseconds(2500))
			done = true;

		if (!done && m_inv_fid && m_main_fid && m_cur_fid && m_getitem_mid && m_getid_mid)
		{
			auto inv = env->GetObjectField(player, m_inv_fid);
			if (inv)
			{
				jint cur = env->GetIntField(inv, m_cur_fid);

				if (cur != (jint)m_soup_slot)
				{

					done = true;
				}
				else
				{
					auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
					if (arr)
					{
						auto stack = env->GetObjectArrayElement(arr, cur);
						bool still_soup = false;

						if (stack)
						{
							auto item = env->CallObjectMethod(stack, m_getitem_mid);
							if (env->ExceptionCheck()) env->ExceptionClear();

							if (item)
							{
								jint id = env->CallStaticIntMethod(m_item_class, m_getid_mid, item);
								if (env->ExceptionCheck()) { env->ExceptionClear(); id = -1; }
								still_soup = (id == 282);
								env->DeleteLocalRef(item);
							}
							env->DeleteLocalRef(stack);
						}
						else if (env->ExceptionCheck())
						{
							env->ExceptionClear();
						}

						if (!still_soup)
							done = true;

						env->DeleteLocalRef(arr);
					}
				}

				env->DeleteLocalRef(inv);
			}
		}

		if (done)
			finish(ctx);
	}

	void on_disable(std::shared_ptr<c_context> ctx) override
	{
		finish(ctx);
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Health"), &m_health, 4.0f, 18.0f, xorstr_("%.0f"));
		ImGui::Dummy(ImVec2(1, 4));
		ui::toggle(xorstr_("Drop bowl"), &m_drop_bowl);
	}

	void save_config(config_data& d) override
	{
		d.set_float(xorstr_("as_health"), m_health);
		d.set_int(xorstr_("as_drop_bowl"), m_drop_bowl);
	}

	void load_config(const config_data& d) override
	{
		m_health = d.get_float(xorstr_("as_health"), 12.0f);
		m_drop_bowl = d.get_int(xorstr_("as_drop_bowl"), 1);
	}
};
