#include "../headers/wtap.h"

void c_wtap::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->local->get_object())
		return;

	bool lmb = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
	bool w_held = GetAsyncKeyState('W') & 0x8000;

	if (!lmb || !w_held || !ctx->ingame)
	{
		m_resetting = false;
		return;
	}

	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_reset).count();

	int cycle_ms = m_interval * 50;

	if (m_resetting)
	{
		if (elapsed >= 50)
		{
			ctx->local->set_sprinting(true);
			m_resetting = false;
			m_last_reset = now;
		}
		else
		{
			ctx->local->set_sprinting(false);
		}
	}
	else
	{
		if (elapsed >= cycle_ms)
		{
			ctx->local->set_sprinting(false);
			m_resetting = true;
			m_last_reset = now;
		}
	}
}

void c_wtap::on_render()
{
	ImGui::SliderInt(xorstr_("Interval (ticks)"), &m_interval, 4, 20);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(xorstr_("Game ticks between sprint resets (1 tick = 50ms)"));
}

void c_wtap::save_config(config_data& data)
{
	data.set_int(xorstr_("wtap_interval"), m_interval);
}

void c_wtap::load_config(const config_data& data)
{
	m_interval = data.get_int(xorstr_("wtap_interval"), 10);
}
