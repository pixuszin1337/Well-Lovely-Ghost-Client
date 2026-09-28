#include "../headers/triggerbot.h"
#include "../headers/friends.h"
#include "../headers/teams.h"
#include "../headers/antibot.h"
#include <random>

void c_triggerbot::randomize_delay()
{
	static std::mt19937 rng(std::random_device{}());
	int lo = (std::min)(m_min_cps, m_max_cps);
	int hi = (std::max)(m_min_cps, m_max_cps);
	if (lo < 1) lo = 1;
	if (hi < 1) hi = 1;
	std::uniform_int_distribution<int> dist(lo, hi);
	int cps = dist(rng);
	m_current_delay_ms = 1000 / cps;
}

void c_triggerbot::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->minecraft)
		return;

	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_click).count();

	if (elapsed < m_current_delay_ms)
		return;

	auto target = sdk::instance->get_entity_over(ctx->minecraft);
	if (!target)
		return;

	auto env = welllovely::instance->get_env();
	{
		c_player tmp(env->NewLocalRef(target));
		if (c_friends::is_friend(tmp.get_name())
			|| c_teams::is_teammate(tmp.get_object())
			|| c_antibot::is_bot(&tmp))
		{
			env->DeleteLocalRef(target);
			return;
		}
	}

	env->DeleteLocalRef(target);

	sdk::instance->click_mouse(ctx->minecraft);
	m_last_click = now;
	randomize_delay();
}

void c_triggerbot::on_render()
{
	ImGui::SliderInt(xorstr_("Min CPS"), &m_min_cps, 1, 20);
	ImGui::SliderInt(xorstr_("Max CPS"), &m_max_cps, 1, 20);
}

void c_triggerbot::save_config(config_data& data)
{
	data.set_int(xorstr_("triggerbot_min_cps"), m_min_cps);
	data.set_int(xorstr_("triggerbot_max_cps"), m_max_cps);
}

void c_triggerbot::load_config(const config_data& data)
{
	m_min_cps = data.get_int(xorstr_("triggerbot_min_cps"), 8);
	m_max_cps = data.get_int(xorstr_("triggerbot_max_cps"), 12);
}
