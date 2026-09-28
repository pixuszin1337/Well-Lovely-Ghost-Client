#include "../headers/aimbot.h"
#include "../headers/friends.h"
#include "../headers/teams.h"
#include "../headers/antibot.h"
#include "../../../../sdk/input/input_hook.h"

void c_aimbot::on_draw(ImDrawList* )
{
	auto env = welllovely::instance->get_env();
	if (!env)
		return;

	if (env->ExceptionCheck())
		env->ExceptionClear();

	auto mc = sdk::instance->get_minecraft();
	if (!mc)
		return;

	auto now = std::chrono::steady_clock::now();
	float dt = std::chrono::duration<float>(now - m_last).count();
	m_last = now;
	if (dt > 0.1f) dt = 0.1f;

	auto local = std::make_shared<c_player>(sdk::instance->get_player(mc));
	auto world = std::make_shared<c_world>(sdk::instance->get_world(mc));

	auto screen = sdk::instance->get_current_screen(mc);
	bool ingame = !screen;
	if (screen)
		env->DeleteLocalRef(screen);

	bool attacking_ok = !m_only_attacking || input_hook::is_lmb_down()
	                 || (wrapper::get_async_keystate(VK_LBUTTON) & 0x8000);
	bool block_ok = !m_block_check || !sdk::instance->is_looking_at_block(mc);

	if (ingame && attacking_ok && block_ok && local->get_object() && world->get_object())
	{

		std::shared_ptr<c_player> target = nullptr;
		double best = 1e18;

		for (auto& p : world->get_players())
		{
			if (env->IsSameObject(local->get_object(), p->get_object()))
				continue;
			if (p->get_health() <= 0)
				continue;
			if (m_ignore_invisible && p->is_invisible())
				continue;
			if (c_friends::is_friend(p->get_name()))
				continue;
			if (c_teams::is_teammate(p->get_object()) || c_antibot::is_bot(p.get()))
				continue;

			double d = local->get_distance_to(p);
				if (m_max_distance > 0.0f && d > m_max_distance)
					continue;
			if (d < best)
			{
				best = d;
				target = p;
			}
		}

		if (target)
		{
			auto angles = sdk::util::get_angles(local->get_position(), target->get_position());
			float difference = sdk::util::wrap_to_180(-(local->get_yaw() - angles.first));

			if (abs(difference) <= m_fov)
			{
				float factor = m_speed * auth::decode_float(m_aim_factor, auth::session_key()) * dt;
				if (factor > 1.0f)
					factor = 1.0f;

				local->add_yaw(difference * factor);
			}
		}
	}

	env->DeleteLocalRef(mc);
}

void c_aimbot::on_render()
{
	ui::slider_float(xorstr_("Speed"), &m_speed, 1.0f, 10.0f, xorstr_("%.1f"));
	ui::slider_float(xorstr_("FOV"), &m_fov, 10.0f, 90.0f, xorstr_("%.1f"));
	ui::slider_float(xorstr_("Max distance"), &m_max_distance, 0.0f, 10.0f, xorstr_("%.1f"));
	ui::toggle(xorstr_("Only while attacking"), &m_only_attacking);
	ui::toggle(xorstr_("Block check"), &m_block_check);
	ui::toggle(xorstr_("Ignore invisibles"), &m_ignore_invisible);
}
