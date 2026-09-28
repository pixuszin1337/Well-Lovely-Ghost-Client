#include "../headers/autoclicker.h"
#include "../../../../sdk/input/input_hook.h"

void c_autoclicker::on_tick(std::shared_ptr<c_context> ctx)
{
	if (m_click_pending) {
		bool should_release = m_up_timer.has_passed(m_up_delay)
			|| !ctx->local->get_object()
			|| !ctx->ingame
			|| welllovely::instance->b_open
			|| !input_hook::is_lmb_down();

		if (should_release) {
			INPUT inp{};
			inp.type = INPUT_MOUSE;
			inp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, &inp, sizeof(INPUT));
			m_click_pending = false;
		}
		return;
	}

	if (!ctx->local->get_object() || !ctx->ingame)
		return;

	if (welllovely::instance->b_open)
		return;

	if (!input_hook::is_lmb_down())
		return;

	if (m_block_check && sdk::instance->is_looking_at_block(ctx->minecraft))
		return;

	if (!m_timer.has_passed(m_next_delay))
		return;

	INPUT inp{};
	inp.type = INPUT_MOUSE;
	inp.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &inp, sizeof(INPUT));

	m_click_pending = true;
	m_up_timer.reset();

	float ms = auth::decode_float(m_ms_base, auth::session_key());
	if (m_mode == 0)
	{
		m_next_delay = static_cast<long>(ms / m_cps);
	}
	else
	{
		float lo = ms / m_max_cps;
		float hi = ms / m_min_cps;
		if (lo > hi) { float t = lo; lo = hi; hi = t; }
		std::uniform_real_distribution<float> dist(lo, hi);
		m_next_delay = static_cast<long>(dist(m_rng));
	}

	m_up_delay = (std::min)(40L, m_next_delay / 2);

	m_timer.reset();
}

void c_autoclicker::on_render()
{
	ImGui::RadioButton(xorstr_("Blatant"), &m_mode, 0);
	ImGui::SameLine();
	ImGui::RadioButton(xorstr_("Legit"), &m_mode, 1);

	if (m_mode == 0)
	{
		ui::slider_float(xorstr_("CPS"), &m_cps, 1.0f, 20.0f, xorstr_("%.0f"));
	}
	else
	{
		if (ui::slider_float(xorstr_("Min CPS"), &m_min_cps, 1.0f, 20.0f, xorstr_("%.0f")))
			if (m_min_cps > m_max_cps)
				m_max_cps = m_min_cps;
		if (ui::slider_float(xorstr_("Max CPS"), &m_max_cps, 1.0f, 20.0f, xorstr_("%.0f")))
			if (m_max_cps < m_min_cps)
				m_min_cps = m_max_cps;
	}

	ui::toggle(xorstr_("Block check"), &m_block_check);
}
