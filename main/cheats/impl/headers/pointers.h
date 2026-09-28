#pragma once

#include "impl.h"
#include "../../module.h"
#include "friends.h"
#include "teams.h"
#include "antibot.h"
#include "../../../../sdk/render/render.h"
#include <cmath>

class c_pointers : public c_module {
	sdk::c_render m_render;

public:
	c_pointers() : c_module(xorstr_("Pointers"), e_category::render) {}

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

		float partial = sdk::instance->get_partial_ticks(mc);

		auto local = std::make_shared<c_player>(sdk::instance->get_player(mc));
		auto world = std::make_shared<c_world>(sdk::instance->get_world(mc));

		if (!local->get_object() || !world->get_object())
		{
			env->DeleteLocalRef(mc);
			return;
		}

		float yaw = local->get_yaw();
		auto cam = view.render_pos;

		auto& io = ImGui::GetIO();
		const float cx = io.DisplaySize.x * 0.5f;
		const float cy = io.DisplaySize.y * 0.5f;
		const float rx = cx - 50.0f;
		const float ry = cy - 50.0f;

		for (auto& player : world->get_players())
		{
			if (env->IsSameObject(local->get_object(), player->get_object()))
				continue;

			float hp = player->get_health();
			if (hp <= 0)
				continue;

			if (c_antibot::is_bot(player.get()))
				continue;

			auto pos = player->get_position();
			auto last = player->get_last_tick_pos();
			sdk::vec3d ip{
				last.x + (pos.x - last.x) * partial,
				last.y + (pos.y - last.y) * partial,
				last.z + (pos.z - last.z) * partial
			};

			sdk::vec3d rel{ ip.x - cam.x, ip.y - cam.y, ip.z - cam.z };

			float sx, sy;
			if (sdk::util::world_to_screen(rel, view.modelview, view.projection, view.viewport, sx, sy))
			{
				if (sx >= 0.0f && sx <= io.DisplaySize.x && sy >= 0.0f && sy <= io.DisplaySize.y)
					continue;
			}

			const float fyaw = yaw * 3.14159265358979f / 180.0f;
			const float fwd_x = -std::sin(fyaw), fwd_z = std::cos(fyaw);
			const float rgt_x = -std::cos(fyaw), rgt_z = -std::sin(fyaw);

			const float d_right = (float)rel.x * rgt_x + (float)rel.z * rgt_z;
			const float d_fwd = (float)rel.x * fwd_x + (float)rel.z * fwd_z;
			const float theta = std::atan2(d_right, d_fwd);

			const float dir_x = std::sin(theta);
			const float dir_y = -std::cos(theta);

			const float px = cx + dir_x * rx;
			const float py = cy + dir_y * ry;

			const float dist = std::sqrt((float)(rel.x * rel.x + rel.y * rel.y + rel.z * rel.z));
			float t = dist / 48.0f;
			if (t > 1.0f) t = 1.0f;

			ImU32 col;
			if (c_friends::is_friend(player->get_name()))
			{
				col = IM_COL32(100, 255, 100, 255);
			}
			else if (c_teams::is_teammate(player->get_object()))
			{
				col = IM_COL32(90, 170, 255, 255);
			}
			else
			{
				const int r = (int)(243 + (90.f - 243.f) * t);
				const int g = (int)(139 + (170.f - 139.f) * t);
				const int b = (int)(170 + (255.f - 170.f) * t);
				col = IM_COL32(r, g, b, 255);
			}

			const float perp_x = -dir_y, perp_y = dir_x;
			const ImVec2 tip(px + dir_x * 10.0f, py + dir_y * 10.0f);
			const ImVec2 b1(px - dir_x * 4.0f + perp_x * 5.0f, py - dir_y * 4.0f + perp_y * 5.0f);
			const ImVec2 b2(px - dir_x * 4.0f - perp_x * 5.0f, py - dir_y * 4.0f - perp_y * 5.0f);

			draw->AddTriangle(tip, b1, b2, IM_COL32(0, 0, 0, 180), 2.0f);
			draw->AddTriangleFilled(tip, b1, b2, col);
		}
	}
};
