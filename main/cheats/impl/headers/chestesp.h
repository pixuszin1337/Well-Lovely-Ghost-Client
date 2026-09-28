#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/render/render.h"
#include <chrono>
#include <vector>
#include <array>

class c_chestesp : public c_module {
	sdk::c_render m_render;

	std::chrono::steady_clock::time_point m_last_scan{};
	std::vector<std::array<float, 3>> m_cache;

	jfieldID m_list_fid = nullptr;
	jfieldID m_pos_fid = nullptr;
	jmethodID m_getx_mid = nullptr;
	jmethodID m_gety_mid = nullptr;
	jmethodID m_getz_mid = nullptr;
	jclass m_chest_cls = nullptr;
	jclass m_list_cls = nullptr;
	jmethodID m_toarray_mid = nullptr;
	bool m_resolved = false;

public:
	c_chestesp() : c_module(xorstr_("Chest ESP"), e_category::render) {}

	void on_draw(ImDrawList* draw) override
	{
		auto env = welllovely::instance->get_env();
		if (!env)
			return;

		auto mc = sdk::instance->get_minecraft();
		if (!mc)
			return;

		auto now = std::chrono::steady_clock::now();

		if (now - m_last_scan < std::chrono::milliseconds(500))
		{
			draw_cache(draw, mc, env);
			return;
		}
		m_last_scan = now;

		m_cache.clear();

		auto world_obj = sdk::instance->get_world(mc);
		if (!world_obj) { env->DeleteLocalRef(mc); return; }

		if (!m_resolved)
		{
			m_resolved = true;

			auto w_cls = env->GetObjectClass(world_obj);
			m_list_fid = env->GetFieldID(w_cls,
				CUSTOM_CLIENT ? xorstr_("loadedTileEntityList") : BADLION_CLIENT ? xorstr_("c") : xorstr_("field_147482_g"),
				xorstr_("Ljava/util/List;"));
			env->DeleteLocalRef(w_cls);

			if (!m_list_fid) { env->ExceptionClear(); env->DeleteLocalRef(world_obj); env->DeleteLocalRef(mc); return; }

			auto local_chest = welllovely::instance->find_class_quiet(
				!BADLION_CLIENT ? xorstr_("net.minecraft.tileentity.TileEntityChest") : xorstr_("aqk"));
			if (local_chest)
			{
				m_chest_cls = (jclass)env->NewGlobalRef(local_chest);
				env->DeleteLocalRef(local_chest);
			}

			auto local_te = welllovely::instance->find_class_quiet(
				!BADLION_CLIENT ? xorstr_("net.minecraft.tileentity.TileEntity") : xorstr_("aqi"));
			if (local_te)
			{

				m_pos_fid = env->GetFieldID(local_te,
					CUSTOM_CLIENT ? xorstr_("pos") : BADLION_CLIENT ? xorstr_("b") : xorstr_("field_174879_c"),
					!BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/BlockPos;") : xorstr_("Ldt;"));
				if (!m_pos_fid && env->ExceptionCheck()) env->ExceptionClear();

				if (m_pos_fid)
				{

					auto bp_cls = welllovely::instance->find_class_quiet(
						!BADLION_CLIENT ? xorstr_("net.minecraft.util.BlockPos") : xorstr_("dt"));
					if (bp_cls)
					{
						m_getx_mid = env->GetMethodID(bp_cls, CUSTOM_CLIENT ? xorstr_("getX") : BADLION_CLIENT ? xorstr_("d") : xorstr_("func_177958_n"), xorstr_("()I"));
						if (!m_getx_mid && env->ExceptionCheck()) env->ExceptionClear();
						m_gety_mid = env->GetMethodID(bp_cls, CUSTOM_CLIENT ? xorstr_("getY") : BADLION_CLIENT ? xorstr_("e") : xorstr_("func_177956_o"), xorstr_("()I"));
						if (!m_gety_mid && env->ExceptionCheck()) env->ExceptionClear();
						m_getz_mid = env->GetMethodID(bp_cls, CUSTOM_CLIENT ? xorstr_("getZ") : BADLION_CLIENT ? xorstr_("f") : xorstr_("func_177952_p"), xorstr_("()I"));
						if (!m_getz_mid && env->ExceptionCheck()) env->ExceptionClear();

						env->DeleteLocalRef(bp_cls);
					}
				}

				env->DeleteLocalRef(local_te);
			}

			if (!m_chest_cls || !m_pos_fid || !m_getx_mid || !m_gety_mid || !m_getz_mid)
			{
				env->DeleteLocalRef(world_obj);
				env->DeleteLocalRef(mc);
				return;
			}
		}

		auto list = env->GetObjectField(world_obj, m_list_fid);
		env->DeleteLocalRef(world_obj);
		if (!list) { env->DeleteLocalRef(mc); return; }

		if (!m_toarray_mid)
		{
			auto local_list = env->FindClass(xorstr_("java/util/List"));
			if (local_list)
			{
				m_toarray_mid = env->GetMethodID(local_list, xorstr_("toArray"), xorstr_("()[Ljava/lang/Object;"));
				if (!m_toarray_mid && env->ExceptionCheck()) env->ExceptionClear();
				env->DeleteLocalRef(local_list);
			}
		}

		if (m_toarray_mid)
		{
			auto arr = reinterpret_cast<jobjectArray>(env->CallObjectMethod(list, m_toarray_mid));
			if (arr)
			{
				jsize len = env->GetArrayLength(arr);

				for (jsize i = 0; i < len; i++)
				{
					auto te = env->GetObjectArrayElement(arr, i);
					if (!te)
					{
						if (env->ExceptionCheck()) env->ExceptionClear();
						continue;
					}

					if (env->IsInstanceOf(te, m_chest_cls))
					{
						auto bp = env->GetObjectField(te, m_pos_fid);
						if (bp)
						{
							m_cache.push_back({
								(float)env->CallIntMethod(bp, m_getx_mid),
								(float)env->CallIntMethod(bp, m_gety_mid),
								(float)env->CallIntMethod(bp, m_getz_mid)
							});
							env->DeleteLocalRef(bp);
						}
						else if (env->ExceptionCheck())
						{
							env->ExceptionClear();
						}
					}

					env->DeleteLocalRef(te);
				}

				env->DeleteLocalRef(arr);
			}
			else if (env->ExceptionCheck())
			{
				env->ExceptionClear();
			}
		}

		env->DeleteLocalRef(list);

		draw_cache(draw, mc, env);
	}

private:
	void draw_cache(ImDrawList* draw, jobject mc, JNIEnv* env)
	{
		if (m_cache.empty())
		{
			env->DeleteLocalRef(mc);
			return;
		}

		auto view = m_render.capture();
		if (!view.valid)
		{
			env->DeleteLocalRef(mc);
			return;
		}

		for (auto& c : m_cache)
		{
			auto box = sdk::project_box(view, sdk::vec3d{ c[0], c[1], c[2] }, 1.0, 1.0);

			if (box.box_valid)
			{
				draw->AddRect(ImVec2(box.min_x - 1.f, box.min_y - 1.f), ImVec2(box.max_x + 1.f, box.max_y + 1.f), IM_COL32(0, 0, 0, 180), 0.f, 0, 3.f);
				draw->AddRect(ImVec2(box.min_x, box.min_y), ImVec2(box.max_x, box.max_y), IM_COL32(255, 170, 40, 235), 0.f, 0, 1.5f);
			}
		}

		env->DeleteLocalRef(mc);
	}
};
