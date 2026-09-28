#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/render/render.h"
#include <chrono>
#include <vector>
#include <array>

class c_itemesp : public c_module {
	sdk::c_render m_render;

	std::chrono::steady_clock::time_point m_last_scan{};
	std::vector<std::array<float, 3>> m_cache;

	jfieldID m_list_fid = nullptr;
	jfieldID m_px_fid = nullptr;
	jfieldID m_py_fid = nullptr;
	jfieldID m_pz_fid = nullptr;
	jclass m_itement_cls = nullptr;
	jmethodID m_toarray_mid = nullptr;
	bool m_resolved = false;

public:
	c_itemesp() : c_module(xorstr_("Item ESP"), e_category::render) {}

	void on_draw(ImDrawList* draw) override
	{
		auto env = welllovely::instance->get_env();
		if (!env)
			return;

		auto mc = sdk::instance->get_minecraft();
		if (!mc)
			return;

		auto now = std::chrono::steady_clock::now();

		if (now - m_last_scan < std::chrono::milliseconds(300))
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
				CUSTOM_CLIENT ? xorstr_("loadedEntityList") : BADLION_CLIENT ? xorstr_("n") : xorstr_("field_72996_f"),
				xorstr_("Ljava/util/List;"));
			env->DeleteLocalRef(w_cls);

			if (!m_list_fid) { env->ExceptionClear(); env->DeleteLocalRef(world_obj); env->DeleteLocalRef(mc); return; }

			auto local_ie = welllovely::instance->find_class_quiet(
				!BADLION_CLIENT ? xorstr_("net.minecraft.entity.item.EntityItem") : xorstr_("abt"));
			if (local_ie)
			{
				m_itement_cls = (jclass)env->NewGlobalRef(local_ie);

				m_px_fid = env->GetFieldID(local_ie,
					CUSTOM_CLIENT ? xorstr_("posX") : BADLION_CLIENT ? xorstr_("s") : xorstr_("field_70165_t"),
					xorstr_("D"));
				if (!m_px_fid && env->ExceptionCheck()) env->ExceptionClear();

				m_py_fid = env->GetFieldID(local_ie,
					CUSTOM_CLIENT ? xorstr_("posY") : BADLION_CLIENT ? xorstr_("t") : xorstr_("field_70163_u"),
					xorstr_("D"));
				if (!m_py_fid && env->ExceptionCheck()) env->ExceptionClear();

				m_pz_fid = env->GetFieldID(local_ie,
					CUSTOM_CLIENT ? xorstr_("posZ") : BADLION_CLIENT ? xorstr_("u") : xorstr_("field_70161_v"),
					xorstr_("D"));
				if (!m_pz_fid && env->ExceptionCheck()) env->ExceptionClear();

				env->DeleteLocalRef(local_ie);
			}

			if (!m_itement_cls || !m_px_fid || !m_py_fid || !m_pz_fid)
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
					auto e = env->GetObjectArrayElement(arr, i);
					if (!e)
					{
						if (env->ExceptionCheck()) env->ExceptionClear();
						continue;
					}

					if (env->IsInstanceOf(e, m_itement_cls))
					{
						m_cache.push_back({
							(float)env->GetDoubleField(e, m_px_fid),
							(float)env->GetDoubleField(e, m_py_fid),
							(float)env->GetDoubleField(e, m_pz_fid)
						});
					}

					env->DeleteLocalRef(e);
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

			auto box = sdk::project_box(view, sdk::vec3d{ c[0], c[1] - 0.125, c[2] }, 0.25, 0.25);

			if (box.box_valid)
			{
				draw->AddRect(ImVec2(box.min_x - 1.f, box.min_y - 1.f), ImVec2(box.max_x + 1.f, box.max_y + 1.f), IM_COL32(0, 0, 0, 160), 0.f, 0, 2.f);
				draw->AddRect(ImVec2(box.min_x, box.min_y), ImVec2(box.max_x, box.max_y), IM_COL32(255, 215, 90, 225), 0.f, 0, 1.f);
			}
		}

		env->DeleteLocalRef(mc);
	}
};
