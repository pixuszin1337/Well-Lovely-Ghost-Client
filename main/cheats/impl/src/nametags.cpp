#include "../headers/nametags.h"
#include "../headers/friends.h"

void c_nametags::on_draw(ImDrawList* draw)
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
			if (player->get_health() <= 0)
				continue;

			auto pos = player->get_position();
			auto last = player->get_last_tick_pos();
			sdk::vec3d ip{
				last.x + (pos.x - last.x) * partial,
				last.y + (pos.y - last.y) * partial,
				last.z + (pos.z - last.z) * partial
			};

			auto box = sdk::project_box(view, ip, 0.6, 1.8);
			if (!box.head_valid)
				continue;

			std::string name = player->get_name();
			if (name.empty())
				continue;

			const ImVec2 sz = ImGui::CalcTextSize(name.c_str());
			const float tx = box.head_x - sz.x * 0.5f;
			const float ty = box.head_y - sz.y - 2.0f;

			const ImU32 name_col = c_friends::is_friend(name)
				? IM_COL32(100, 255, 100, 255)
				: IM_COL32(255, 255, 255, 255);
			draw->AddText(ImVec2(tx + 1.f, ty + 1.f), IM_COL32(0, 0, 0, 200), name.c_str());
			draw->AddText(ImVec2(tx, ty), name_col, name.c_str());
		}
	}

	env->DeleteLocalRef(mc);
}
