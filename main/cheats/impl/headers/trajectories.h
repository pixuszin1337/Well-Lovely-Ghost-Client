#pragma once

#include "impl.h"
#include "../../module.h"
#include "../../../../sdk/render/render.h"
#include <cmath>
#include <vector>

class c_trajectories : public c_module {
	sdk::c_render m_render;
	bool m_always_show_bow = false;
	bool m_landing_box = true;

	jfieldID m_inv_fid = nullptr;
	jfieldID m_main_fid = nullptr;
	jfieldID m_cur_fid = nullptr;
	jfieldID m_iteminuse_fid = nullptr;
	jfieldID m_usecount_fid = nullptr;
	jmethodID m_getitem_mid = nullptr;
	jmethodID m_getmeta_mid = nullptr;
	jmethodID m_getid_mid = nullptr;
	jclass m_item_class = nullptr;
	bool m_resolved = false;

	struct projectile_type { float gravity, drag, velocity; };

public:
	c_trajectories() : c_module(xorstr_("Trajectories"), e_category::render) {}

	void on_draw(ImDrawList* draw) override
	{
		auto env = welllovely::instance->get_env();
		if (!env)
			return;

		auto mc = sdk::instance->get_minecraft();
		if (!mc)
			return;

		auto view = m_render.capture();
		if (!view.valid)
		{
			env->DeleteLocalRef(mc);
			return;
		}

		auto local = std::make_shared<c_player>(sdk::instance->get_player(mc));
		auto world = std::make_shared<c_world>(sdk::instance->get_world(mc));

		if (!local->get_object() || !world->get_object())
		{
			env->DeleteLocalRef(mc);
			return;
		}

		if (!m_resolved)
		{
			m_resolved = true;

			auto p_cls = env->GetObjectClass(local->get_object());
			m_inv_fid = env->GetFieldID(p_cls,
				CUSTOM_CLIENT ? xorstr_("inventory") : BADLION_CLIENT ? xorstr_("bo") : xorstr_("field_71071_by"),
				!BADLION_CLIENT ? xorstr_("Lnet/minecraft/entity/player/InventoryPlayer;") : xorstr_("Ladz;"));
			if (!m_inv_fid && env->ExceptionCheck()) env->ExceptionClear();

			m_iteminuse_fid = env->GetFieldID(p_cls,
				CUSTOM_CLIENT ? xorstr_("itemInUse") : BADLION_CLIENT ? xorstr_("aT") : xorstr_("field_71074_e"),
				!BADLION_CLIENT ? xorstr_("Lnet/minecraft/item/ItemStack;") : xorstr_("Ladd;"));
			if (!m_iteminuse_fid && env->ExceptionCheck()) env->ExceptionClear();

			m_usecount_fid = env->GetFieldID(p_cls,
				CUSTOM_CLIENT ? xorstr_("itemInUseCount") : BADLION_CLIENT ? xorstr_("aS") : xorstr_("field_71072_f"),
				xorstr_("I"));
			if (!m_usecount_fid && env->ExceptionCheck()) env->ExceptionClear();

			env->DeleteLocalRef(p_cls);

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
		}

		if (!m_inv_fid || !m_getitem_mid || !m_getid_mid || !m_item_class)
		{
			env->DeleteLocalRef(mc);
			return;
		}

		auto inv = env->GetObjectField(local->get_object(), m_inv_fid);
		if (!inv) { env->DeleteLocalRef(mc); return; }

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
		}

		if (!m_main_fid || !m_cur_fid)
		{
			env->DeleteLocalRef(inv);
			env->DeleteLocalRef(mc);
			return;
		}

		jint cur = env->GetIntField(inv, m_cur_fid);
		auto arr = reinterpret_cast<jobjectArray>(env->GetObjectField(inv, m_main_fid));
		env->DeleteLocalRef(inv);

		if (!arr) { env->DeleteLocalRef(mc); return; }

		auto stack = env->GetObjectArrayElement(arr, cur);
		env->DeleteLocalRef(arr);

		if (!stack)
		{
			if (env->ExceptionCheck()) env->ExceptionClear();
			env->DeleteLocalRef(mc);
			return;
		}

		jint item_id = -1;
		jint meta = 0;

		auto item = env->CallObjectMethod(stack, m_getitem_mid);
		if (env->ExceptionCheck()) env->ExceptionClear();

		if (item)
		{
			item_id = env->CallStaticIntMethod(m_item_class, m_getid_mid, item);
			if (env->ExceptionCheck()) { env->ExceptionClear(); item_id = -1; }
			env->DeleteLocalRef(item);
		}

		if (m_getmeta_mid)
		{
			meta = env->CallIntMethod(stack, m_getmeta_mid);
			if (env->ExceptionCheck()) { env->ExceptionClear(); meta = 0; }
		}

		env->DeleteLocalRef(stack);

		bool valid = false;
		projectile_type type{};

		if (item_id == 261)
		{
			jint use_count = 0;
			bool using_item = false;

			if (m_iteminuse_fid)
			{
				auto in_use = env->GetObjectField(local->get_object(), m_iteminuse_fid);
				if (env->ExceptionCheck()) env->ExceptionClear();
				using_item = in_use != nullptr;
				if (in_use) env->DeleteLocalRef(in_use);
			}

			if (m_usecount_fid && using_item)
				use_count = env->GetIntField(local->get_object(), m_usecount_fid);

			if (using_item || m_always_show_bow)
			{
				float charge = (float)use_count / 20.0f;
				float power = charge * charge + charge * 2.0f;
				if (power > 1.0f) power = 1.0f;
				if (power < 0.1f) power = 1.0f;

				type = { 0.05f, 0.99f, power * 3.0f };
				valid = true;
			}
		}
		else if (item_id == 332 || item_id == 344 || item_id == 368)
		{

			type = { 0.03f, 0.99f, 1.5f };
			valid = true;
		}
		else if (item_id == 373 && (meta & 0x4000))
		{

			type = { 0.05f, 0.99f, 0.75f };
			valid = true;
		}
		else if (item_id == 346)
		{

			type = { 0.03f, 0.92f, 1.5f };
			valid = true;
		}

		if (!valid)
		{
			env->DeleteLocalRef(mc);
			return;
		}

		float yaw = local->get_yaw();
		float pitch = local->get_pitch();

		float yaw_rad = yaw * 3.14159265358979f / 180.0f;
		float pitch_rad = pitch * 3.14159265358979f / 180.0f;

		double motion_x = -(double)std::sin(yaw_rad) * std::cos(pitch_rad) * type.velocity;
		double motion_y = -(double)std::sin(pitch_rad) * type.velocity;
		double motion_z = (double)std::cos(yaw_rad) * std::cos(pitch_rad) * type.velocity;

		auto pos = local->get_position();
		double eye_x = pos.x, eye_y = pos.y + 1.62, eye_z = pos.z;

		double x = eye_x - (double)std::cos(yaw_rad) * 0.16;
		double y = eye_y - 0.10000000149011612;
		double z = eye_z - (double)std::sin(yaw_rad) * 0.16;

		std::vector<sdk::vec3d> points;
		points.push_back({ x, y, z });

		bool landed = false;
		sdk::vec3d landing{};

		for (int t = 0; t < 240; t++)
		{
			double nx = x + motion_x;
			double ny = y + motion_y;
			double nz = z + motion_z;

			if (!world->is_air_block((int)std::floor(nx), (int)std::floor(ny), (int)std::floor(nz)))
			{
				landing = { nx, ny, nz };
				landed = true;
				points.push_back(landing);
				break;
			}

			x = nx; y = ny; z = nz;

			motion_x *= type.drag;
			motion_y *= type.drag;
			motion_z *= type.drag;
			motion_y -= type.gravity;

			points.push_back({ x, y, z });

			double dx = x - eye_x, dy = y - eye_y, dz = z - eye_z;
			if (dx * dx + dy * dy + dz * dz > 96.0 * 96.0)
				break;
			if (y < -64.0)
				break;
		}

		std::vector<ImVec2> screen;
		screen.reserve(points.size());

		for (auto& p : points)
		{
			sdk::vec3d rel{ p.x - view.render_pos.x, p.y - view.render_pos.y, p.z - view.render_pos.z };
			float sx, sy;
			if (sdk::util::world_to_screen(rel, view.modelview, view.projection, view.viewport, sx, sy))
				screen.push_back(ImVec2(sx, sy));
		}

		if (screen.size() >= 2)
		{
			draw->AddPolyline(screen.data(), (int)screen.size(),
				IM_COL32(255, 255, 255, 200), 0, 2.0f);

			draw->AddCircleFilled(screen.back(), 4.0f, IM_COL32(255, 255, 255, 230), 12);
			draw->AddCircle(screen.back(), 5.0f, IM_COL32(0, 0, 0, 180), 12, 2.0f);
		}

		if (m_landing_box && landed)
		{
			auto box = sdk::project_box(view,
				sdk::vec3d{ std::floor(landing.x), std::floor(landing.y), std::floor(landing.z) }, 1.0, 1.0);

			if (box.box_valid)
			{
				draw->AddRect(ImVec2(box.min_x - 1.f, box.min_y - 1.f), ImVec2(box.max_x + 1.f, box.max_y + 1.f), IM_COL32(0, 0, 0, 180), 0.f, 0, 3.f);
				draw->AddRect(ImVec2(box.min_x, box.min_y), ImVec2(box.max_x, box.max_y), IM_COL32(0, 160, 255, 200), 0.f, 0, 1.5f);
			}
		}

		env->DeleteLocalRef(mc);
	}

	void on_render() override
	{
		ui::toggle(xorstr_("Always show bow"), &m_always_show_bow);
		ui::toggle(xorstr_("Landing box"), &m_landing_box);
	}

	void save_config(config_data& d) override
	{
		d.set_int(xorstr_("tr_bow"), m_always_show_bow);
		d.set_int(xorstr_("tr_box"), m_landing_box);
	}

	void load_config(const config_data& d) override
	{
		m_always_show_bow = d.get_int(xorstr_("tr_bow"), 0);
		m_landing_box = d.get_int(xorstr_("tr_box"), 1);
	}
};
