#pragma once

#include "impl.h"
#include "../../module.h"
#include "friends.h"
#include "teams.h"
#include "antibot.h"
#include <chrono>
#include <cmath>

class c_blockin : public c_module {
	enum class e_state { scanning, aiming, placing };

	float m_max_distance = 4.5f;
	bool m_cover_head = true;
	int m_place_delay = 250;

	e_state m_state = e_state::scanning;
	float m_saved_yaw = 0.0f, m_saved_pitch = 0.0f;
	bool m_rot_saved = false;
	std::chrono::steady_clock::time_point m_phase{};

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getid_mid = nullptr;
	jclass m_item_class = nullptr;
	bool m_resolved = false;

	bool hotbar_has_block(JNIEnv* env, jobject player)
	{
		if (!m_inv_fid || !m_main_fid || !m_getitem_mid || !m_getid_mid || !m_item_class)
			return false;

		auto inv = env->GetObjectField(player, m_inv_fid);
		if (!inv) return false;

		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
		env->DeleteLocalRef(inv);
		if (!arr) return false;

		jsize len = env->GetArrayLength(arr);
		if (len > 9) len = 9;

		bool found = false;

		for (jsize i = 0; i < len && !found; i++)
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
				found = (id >= 1 && id <= 255);
				env->DeleteLocalRef(item);
			}

			env->DeleteLocalRef(stack);
		}

		env->DeleteLocalRef(arr);
		return found;
	}

	void restore_look(std::shared_ptr<c_context> ctx)
	{
		if (m_rot_saved && ctx->local->get_object())
		{
			ctx->local->set_yaw(m_saved_yaw);
			ctx->local->set_pitch(m_saved_pitch);
		}
		m_rot_saved = false;
	}

public:
	c_blockin() : c_module(xorstr_("Block In"), e_category::block) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object() || !ctx->world)
			return;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();
		auto now = std::chrono::steady_clock::now();

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
		}

		if (!m_inv_fid || !m_getitem_mid || !m_getid_mid || !m_item_class)
			return;

		if (!m_main_fid)
		{
			auto inv = env->GetObjectField(player, m_inv_fid);
			if (!inv) return;

			auto inv_cls = env->GetObjectClass(inv);
			m_main_fid = env->GetFieldID(inv_cls,
				CUSTOM_CLIENT ? xorstr_("mainInventory") : BADLION_CLIENT ? xorstr_("a") : xorstr_("field_70462_a"),
				!BADLION_CLIENT ? xorstr_("[Lnet/minecraft/item/ItemStack;") : xorstr_("[Ladd;"));
			env->DeleteLocalRef(inv_cls);

			if (!m_main_fid) { env->ExceptionClear(); return; }
			env->DeleteLocalRef(inv);
		}

		switch (m_state)
		{
		case e_state::scanning:
		{
			if (now - m_phase < std::chrono::milliseconds(m_place_delay))
				break;
			m_phase = now;

			if (!hotbar_has_block(env, player))
			{
				restore_look(ctx);
				break;
			}

			std::shared_ptr<c_player> target = nullptr;
			double best = (double)m_max_distance;

			auto local = ctx->local;

			for (auto& p : ctx->world->get_players())
			{
				if (env->IsSameObject(player, p->get_object()))
					continue;
				if (p->get_health() <= 0)
					continue;
				if (c_friends::is_friend(p->get_name())
					|| c_teams::is_teammate(p->get_object())
					|| c_antibot::is_bot(p.get()))
					continue;

				double d = local->get_distance_to(p);
				if (d < best)
				{
					best = d;
					target = p;
				}
			}

			if (!target)
			{
				restore_look(ctx);
				break;
			}

			auto tp = target->get_position();
			auto eye = local->get_position();
			eye.y += 1.62;

			const int dx[4] = { 1, -1, 0, 0 };
			const int dz[4] = { 0, 0, 1, -1 };

			int tx = (int)std::floor(tp.x);
			int ty = (int)std::floor(tp.y);
			int tz = (int)std::floor(tp.z);

			bool found = false;
			sdk::vec3d aim{};

			for (int ring = 0; ring < (m_cover_head ? 2 : 1) && !found; ring++)
			{
				for (int i = 0; i < 4 && !found; i++)
				{
					int cx = tx + dx[i], cy = ty + ring, cz = tz + dz[i];

					if (!ctx->world->is_air_block(cx, cy, cz))
						continue;

					double vx = (cx + 0.5) - eye.x;
					double vz = (cz + 0.5) - eye.z;
					double len = std::sqrt(vx * vx + vz * vz);
					if (len < 0.01) continue;

					int sx = (int)std::lround(vx / len);
					int sz = (int)std::lround(vz / len);

					if (ctx->world->is_air_block(cx + sx, cy, cz + sz))
						continue;

					aim = { cx + 0.5 + sx * 0.5, cy + 0.5, cz + 0.5 + sz * 0.5 };
					found = true;
				}
			}

			if (!found)
			{
				restore_look(ctx);
				break;
			}

			if (!m_rot_saved)
			{
				m_saved_yaw = local->get_yaw();
				m_saved_pitch = local->get_pitch();
				m_rot_saved = true;
			}

			auto angles = sdk::util::get_angles(eye, aim);
			local->set_yaw(angles.first);
			local->set_pitch(angles.second);

			m_state = e_state::aiming;
			m_phase = now;
			break;
		}

		case e_state::aiming:
			if (now - m_phase < std::chrono::milliseconds(100))
				break;

			sdk::instance->right_click_mouse(ctx->minecraft);
			m_state = e_state::placing;
			m_phase = now;
			break;

		case e_state::placing:
			if (now - m_phase < std::chrono::milliseconds(60))
				break;

			m_state = e_state::scanning;
			m_phase = now;
			break;
		}
	}

	void on_disable(std::shared_ptr<c_context> ctx) override
	{
		restore_look(ctx);
		m_state = e_state::scanning;
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Range"), &m_max_distance, 2.0f, 6.0f, xorstr_("%.1f"));
		ui::slider_int(xorstr_("Place delay"), &m_place_delay, 100, 600, xorstr_("%d ms"));
		ImGui::Dummy(ImVec2(1, 4));
		ui::toggle(xorstr_("Cover head"), &m_cover_head);
	}

	void save_config(config_data& d) override
	{
		d.set_float(xorstr_("bi_range"), m_max_distance);
		d.set_int(xorstr_("bi_delay"), m_place_delay);
		d.set_int(xorstr_("bi_head"), m_cover_head);
	}

	void load_config(const config_data& d) override
	{
		m_max_distance = d.get_float(xorstr_("bi_range"), 4.5f);
		m_place_delay = d.get_int(xorstr_("bi_delay"), 250);
		m_cover_head = d.get_int(xorstr_("bi_head"), 1);
	}
};
