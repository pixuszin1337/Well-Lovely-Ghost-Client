#include "../headers/tracers.h"
#include "../headers/friends.h"
#include "../headers/antibot.h"

void c_tracers::on_draw(ImDrawList* draw)
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

		const float ox = view.viewport[0] + view.viewport[2] * 0.5f;
		const float oy = static_cast<float>(view.viewport[1] + view.viewport[3]);

		const ImU32 col = IM_COL32(
			static_cast<int>(visuals::esp_color[0] * 255.f), static_cast<int>(visuals::esp_color[1] * 255.f),
			static_cast<int>(visuals::esp_color[2] * 255.f), static_cast<int>(visuals::esp_color[3] * 255.f));

		for (auto& player : world->get_players())
		{
			if (env->IsSameObject(local->get_object(), player->get_object()))
				continue;
			if (player->get_health() <= 0)
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
			if (!box.feet_valid)
				continue;

			bool is_friend = c_friends::is_friend(player->get_name());
			const ImU32 line_col = is_friend ? IM_COL32(100, 255, 100, 255) : col;

			draw->AddLine(ImVec2(ox, oy), ImVec2(box.feet_x, box.feet_y), IM_COL32(0, 0, 0, 180), 3.0f);
			draw->AddLine(ImVec2(ox, oy), ImVec2(box.feet_x, box.feet_y), line_col, 1.5f);
		}
	}

	env->DeleteLocalRef(mc);
}
