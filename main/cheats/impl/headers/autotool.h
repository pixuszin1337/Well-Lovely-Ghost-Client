#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/input/input_hook.h"

class c_autotool : public c_module {
	bool m_switch_back = true;
	int m_return_slot = -1;
	bool m_was_digging = false;

	jfieldID m_mop_fid = nullptr;
	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getpos_mid = nullptr;
	jmethodID m_getstate_mid = nullptr;
	jmethodID m_getblock_mid = nullptr;
	jmethodID m_strvs_mid = nullptr;
	bool m_resolved = false;
	bool m_methods_dead = false;

	std::chrono::steady_clock::time_point m_last_check{};

public:
	c_autotool() : c_module(xorstr_("Auto Tool"), e_category::block) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->minecraft || !ctx->local->get_object() || !ctx->world)
			return;

		auto env = welllovely::instance->get_env();
		auto player = ctx->local->get_object();

		bool digging = input_hook::is_lmb_down();

		if (!digging)
		{
			if (m_was_digging && m_switch_back && m_return_slot >= 0)
			{
				auto inv = env->GetObjectField(player, m_inv_fid);
				if (inv)
				{
					env->SetIntField(inv, m_cur_fid, (jint)m_return_slot);
					env->DeleteLocalRef(inv);
				}
				m_return_slot = -1;
			}
			m_was_digging = false;
			return;
		}
		m_was_digging = true;

		auto now = std::chrono::steady_clock::now();
		if (now - m_last_check < std::chrono::milliseconds(100))
			return;
		m_last_check = now;

		if (!sdk::instance->is_looking_at_block(ctx->minecraft))
			return;

		if (!m_resolved)
		{
			m_resolved = true;

			auto mc_cls = env->GetObjectClass(ctx->minecraft);
			m_mop_fid = env->GetFieldID(mc_cls,
				CUSTOM_CLIENT ? xorstr_("objectMouseOver") : BADLION_CLIENT ? xorstr_("s") : xorstr_("field_71476_x"),
				!BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/MovingObjectPosition;") : xorstr_("Lauh;"));
			env->DeleteLocalRef(mc_cls);
			if (!m_mop_fid) { env->ExceptionClear(); return; }

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

		if (!m_methods_dead && (!m_getitem_mid || !m_strvs_mid || !m_getblock_mid))
		{

			auto stack_cls = welllovely::instance->find_class_quiet(
				!BADLION_CLIENT ? xorstr_("net.minecraft.item.ItemStack") : xorstr_("add"));
			auto state_cls = welllovely::instance->find_class_quiet(
				!BADLION_CLIENT ? xorstr_("net.minecraft.block.state.IBlockState") : xorstr_("atr"));

			if (stack_cls)
			{
				m_getitem_mid = env->GetMethodID(stack_cls,
					CUSTOM_CLIENT ? xorstr_("getItem") : BADLION_CLIENT ? xorstr_("u") : xorstr_("func_77973_b"),
					xorstr_("()Lnet/minecraft/item/Item;"));
				if (!m_getitem_mid && env->ExceptionCheck()) env->ExceptionClear();

				m_strvs_mid = env->GetMethodID(stack_cls,
					CUSTOM_CLIENT ? xorstr_("getStrVsBlock") : BADLION_CLIENT ? xorstr_("a") : xorstr_("func_150997_a"),
					!BADLION_CLIENT ? xorstr_("(Lnet/minecraft/block/Block;)F") : xorstr_("(Lafj;)F"));
				if (!m_strvs_mid && env->ExceptionCheck()) env->ExceptionClear();

				env->DeleteLocalRef(stack_cls);
			}

			if (state_cls)
			{

				m_getblock_mid = env->GetMethodID(state_cls,
					CUSTOM_CLIENT ? xorstr_("getBlock") : BADLION_CLIENT ? xorstr_("d") : xorstr_("func_177230_c"),
					!BADLION_CLIENT ? xorstr_("()Lnet/minecraft/block/Block;") : xorstr_("()Lafj;"));
				if (!m_getblock_mid && env->ExceptionCheck()) env->ExceptionClear();

				env->DeleteLocalRef(state_cls);
			}

			if (!m_getitem_mid || !m_strvs_mid || !m_getblock_mid)
			{
				m_methods_dead = true;
				env->DeleteLocalRef(inv);
				return;
			}
		}

		if (m_methods_dead)
		{
			env->DeleteLocalRef(inv);
			return;
		}

		auto mop = env->GetObjectField(ctx->minecraft, m_mop_fid);
		if (!mop) { env->DeleteLocalRef(inv); return; }

		if (!m_getpos_mid)
		{
			auto mop_cls = env->GetObjectClass(mop);
			m_getpos_mid = env->GetMethodID(mop_cls,
				CUSTOM_CLIENT ? xorstr_("getBlockPos") : BADLION_CLIENT ? xorstr_("a") : xorstr_("func_178782_a"),
				!BADLION_CLIENT ? xorstr_("()Lnet/minecraft/util/BlockPos;") : xorstr_("Ldt;"));
			env->DeleteLocalRef(mop_cls);
			if (!m_getpos_mid) { env->ExceptionClear(); env->DeleteLocalRef(mop); env->DeleteLocalRef(inv); return; }
		}

		auto pos = env->CallObjectMethod(mop, m_getpos_mid);
		if (env->ExceptionCheck() || !pos) { env->ExceptionClear(); env->DeleteLocalRef(mop); env->DeleteLocalRef(inv); return; }

		auto world_obj = ctx->world->get_object();

		if (!m_getstate_mid)
		{
			auto w_cls = env->GetObjectClass(world_obj);
			m_getstate_mid = env->GetMethodID(w_cls,
				CUSTOM_CLIENT ? xorstr_("getBlockState") : BADLION_CLIENT ? xorstr_("p") : xorstr_("func_180495_p"),
				!BADLION_CLIENT ? xorstr_("(Lnet/minecraft/util/BlockPos;)Lnet/minecraft/block/state/IBlockState;") : xorstr_("(Ldt;)Latr;"));
			env->DeleteLocalRef(w_cls);
			if (!m_getstate_mid) { env->ExceptionClear(); env->DeleteLocalRef(pos); env->DeleteLocalRef(mop); env->DeleteLocalRef(inv); return; }
		}

		auto state = env->CallObjectMethod(world_obj, m_getstate_mid, pos);
		if (env->ExceptionCheck() || !state) { env->ExceptionClear(); env->DeleteLocalRef(pos); env->DeleteLocalRef(mop); env->DeleteLocalRef(inv); return; }

		auto block = env->CallObjectMethod(state, m_getblock_mid);
		if (env->ExceptionCheck() || !block) { env->ExceptionClear(); env->DeleteLocalRef(state); env->DeleteLocalRef(pos); env->DeleteLocalRef(mop); env->DeleteLocalRef(inv); return; }

		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
		if (arr)
		{
			jsize len = env->GetArrayLength(arr);
			if (len > 9) len = 9;

			jint cur = env->GetIntField(inv, m_cur_fid);

			int best_slot = -1;
			float best_speed = 0.0f;

			for (jsize i = 0; i < len; i++)
			{
				auto stack = env->GetObjectArrayElement(arr, i);
				if (!stack)
				{
					if (env->ExceptionCheck()) env->ExceptionClear();
					continue;
				}

				float speed = env->CallFloatMethod(stack, m_strvs_mid, block);
				if (env->ExceptionCheck()) { env->ExceptionClear(); speed = 0.0f; }

				env->DeleteLocalRef(stack);

				if (speed > best_speed)
				{
					best_speed = speed;
					best_slot = (int)i;
				}
			}

			if (best_slot >= 0 && best_slot != (int)cur)
			{
				auto cur_stack = env->GetObjectArrayElement(arr, cur);
				if (cur_stack)
				{
					float cur_speed = env->CallFloatMethod(cur_stack, m_strvs_mid, block);
					if (env->ExceptionCheck()) { env->ExceptionClear(); cur_speed = 0.0f; }
					env->DeleteLocalRef(cur_stack);

					if (best_speed > cur_speed + 0.01f)
					{
						if (m_return_slot < 0)
							m_return_slot = (int)cur;
						env->SetIntField(inv, m_cur_fid, (jint)best_slot);
					}
				}
				else
				{
					if (env->ExceptionCheck()) env->ExceptionClear();
					if (m_return_slot < 0)
						m_return_slot = (int)cur;
					env->SetIntField(inv, m_cur_fid, (jint)best_slot);
				}
			}

			env->DeleteLocalRef(arr);
		}

		env->DeleteLocalRef(block);
		env->DeleteLocalRef(state);
		env->DeleteLocalRef(pos);
		env->DeleteLocalRef(mop);
		env->DeleteLocalRef(inv);
	}

	void on_render() override
	{
		ui::toggle(xorstr_("Switch back"), &m_switch_back);
	}

	void save_config(config_data& d) override { d.set_int(xorstr_("at_switchback"), m_switch_back); }
	void load_config(const config_data& d) override { m_switch_back = d.get_int(xorstr_("at_switchback"), 1); }
};
