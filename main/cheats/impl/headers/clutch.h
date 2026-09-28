#pragma once

#include "impl.h"
#include "../../module.h"
#include <chrono>
#include <cmath>

class c_clutch : public c_module {
	enum class e_state { idle, armed, placed };

	e_state m_state = e_state::idle;
	float m_saved_pitch = 0.0f;
	int m_return_slot = -1;
	int m_block_slot = -1;
	std::chrono::steady_clock::time_point m_phase{};

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getid_mid = nullptr;
	jclass m_item_class = nullptr;
	bool m_resolved = false;

	bool find_block_slot(JNIEnv* env, jobject player)
	{
		if (!m_inv_fid) return false;

		auto inv = env->GetObjectField(player, m_inv_fid);
		if (!inv) return false;

		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
		env->DeleteLocalRef(inv);
		if (!arr) return false;

		jsize len = env->GetArrayLength(arr);
		if (len > 9) len = 9;

		int found = -1;

		for (jsize i = 0; i < len && found < 0; i++)
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

				if (id >= 1 && id <= 255)
					found = (int)i;

				env->DeleteLocalRef(item);
			}

			env->DeleteLocalRef(stack);
		}

		env->DeleteLocalRef(arr);
		m_block_slot = found;
		return found >= 0;
	}

	void restore(std::shared_ptr<c_context> ctx)
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

		auto env = welllovely::instance->get_env();

		if (m_block_slot >= 0 && m_return_slot >= 0 && m_inv_fid && m_cur_fid)
		{
			auto inv = env->GetObjectField(ctx->local->get_object(), m_inv_fid);
			if (inv)
			{
				jint cur = env->GetIntField(inv, m_cur_fid);
				if (cur == (jint)m_block_slot)
					env->SetIntField(inv, m_cur_fid, (jint)m_return_slot);
				env->DeleteLocalRef(inv);
			}
		}

		if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
			sdk::instance->right_click_release(ctx->minecraft);

		m_state = e_state::idle;
		m_block_slot = -1;
		m_return_slot = -1;
	}

public:
	c_clutch() : c_module(xorstr_("Clutch"), e_category::block) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object() || !ctx->world)
			return;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();
		auto now = std::chrono::steady_clock::now();

		switch (m_state)
		{
		case e_state::idle:
		{
			if (ctx->local->is_on_ground())
				return;

			auto motion = ctx->local->get_motion_vector();
			if (motion.y > -0.5)
				return;

			if (ctx->local->get_fall_distance() < 3.0f)
				return;

			auto pos = ctx->local->get_position();
			int bx = (int)std::floor(pos.x);
			int bz = (int)std::floor(pos.z);

			double ground = -1.0;
			for (int dy = 0; dy <= 12; dy++)
			{
				int by = (int)std::floor(pos.y) - dy - 1;
				if (!ctx->world->is_air_block(bx, by, bz))
				{
					ground = by + 1.0;
					break;
				}
			}

			if (ground < 0.0)
				return;

			double dist = pos.y - ground;
			if (dist < 0.5 || dist > 2.5)
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

			if (!find_block_slot(env, player))
				return;

			auto inv = env->GetObjectField(player, m_inv_fid);
			if (!inv) return;

			jint cur = env->GetIntField(inv, m_cur_fid);
			m_return_slot = (int)cur;

			if (cur != (jint)m_block_slot)
				env->SetIntField(inv, m_cur_fid, (jint)m_block_slot);

			env->DeleteLocalRef(inv);

			m_saved_pitch = ctx->local->get_pitch();
			ctx->local->set_pitch(90.0f);

			m_state = e_state::armed;
			m_phase = now;
			break;
		}

		case e_state::armed:
			if (now - m_phase < std::chrono::milliseconds(100))
				break;

			sdk::instance->right_click_mouse(ctx->minecraft);
			m_state = e_state::placed;
			m_phase = now;
			break;

		case e_state::placed:
			if (now - m_phase < std::chrono::milliseconds(100))
				break;

			restore(ctx);
			break;
		}
	}

	void on_disable(std::shared_ptr<c_context> ctx) override
	{
		if (m_state != e_state::idle)
			restore(ctx);
	}
};
