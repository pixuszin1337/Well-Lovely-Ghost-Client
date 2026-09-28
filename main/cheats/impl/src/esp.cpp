#include "../headers/esp.h"
#include "../headers/friends.h"
#include "../headers/teams.h"
#include "../headers/antibot.h"

void c_esp::on_draw(ImDrawList* draw)
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

	if (local->get_object() && world->get_object())
	{
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

			auto box = sdk::project_box(view, ip, 0.6, 1.8);

			if (box.box_valid)
			{
				const float minx = box.min_x, miny = box.min_y, maxx = box.max_x, maxy = box.max_y;

				bool is_friend = c_friends::is_friend(player->get_name());
				bool is_team = c_teams::is_teammate(player->get_object());
				const ImU32 boxcol = is_friend
					? IM_COL32(100, 255, 100, 255)
					: is_team
					? IM_COL32(90, 170, 255, 255)
					: IM_COL32(
						static_cast<int>(visuals::esp_color[0] * 255.f), static_cast<int>(visuals::esp_color[1] * 255.f),
						static_cast<int>(visuals::esp_color[2] * 255.f), static_cast<int>(visuals::esp_color[3] * 255.f));

				draw->AddRect(ImVec2(minx - 1.f, miny - 1.f), ImVec2(maxx + 1.f, maxy + 1.f), IM_COL32(0, 0, 0, 180), 0.f, 0, 3.f);
				draw->AddRect(ImVec2(minx, miny), ImVec2(maxx, maxy), boxcol, 0.f, 0, 1.5f);

				if (m_health)
				{
					float frac = hp / 20.0f;
					if (frac < 0.0f) frac = 0.0f;
					if (frac > 1.0f) frac = 1.0f;

					const float bx = minx - 5.0f;
					const float bw = 3.0f;

					draw->AddRectFilled(ImVec2(bx - 1.f, miny - 1.f), ImVec2(bx + bw + 1.f, maxy + 1.f), IM_COL32(0, 0, 0, 180));
					const float fill_top = maxy - (maxy - miny) * frac;
					const ImU32 hpcol = IM_COL32(static_cast<int>((1.0f - frac) * 255.f), static_cast<int>(frac * 235.f), 45, 255);
					draw->AddRectFilled(ImVec2(bx, fill_top), ImVec2(bx + bw, maxy), hpcol);
				}
			}
		}
	}

	env->DeleteLocalRef(mc);
}

void c_esp::on_render()
{
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(xorstr_("Box color"));
	ImGui::SameLine();
	ImGui::ColorEdit4(xorstr_("##espcolor"), visuals::esp_color,
		ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview);

	ui::toggle(xorstr_("Health bar"), &m_health);
}
