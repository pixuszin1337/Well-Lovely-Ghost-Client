#include "../headers/velocity.h"

void c_velocity::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->local->get_object())
		return;

	if (m_space_down) {
		INPUT inp{};
		inp.type = INPUT_KEYBOARD;
		inp.ki.wVk = VK_SPACE;
		inp.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &inp, sizeof(INPUT));
		m_space_down = false;
	}

	const float hurt = ctx->local->get_hurt_time();

	if (hurt > m_prev_hurt)
	{
		auto v = ctx->local->get_motion_vector();
		v.x *= (1.0 - m_horizontal / 100.0);
		v.z *= (1.0 - m_horizontal / 100.0);
		v.y *= (1.0 - m_vertical / 100.0);
		ctx->local->get_motion_vector(v);

		if (m_jump_reset && ctx->local->is_on_ground()) {
			float roll = std::uniform_real_distribution<float>(0.0f, 100.0f)(m_rng);
			if (roll < m_jump_reset_chance) {
				INPUT inp{};
				inp.type = INPUT_KEYBOARD;
				inp.ki.wVk = VK_SPACE;
				SendInput(1, &inp, sizeof(INPUT));
				m_space_down = true;
			}
		}
	}

	m_prev_hurt = hurt;
}

void c_velocity::on_render()
{
	ui::slider_float(xorstr_("Horizontal"), &m_horizontal, 0.0f, 100.0f, xorstr_("%.0f%%"));
	ui::slider_float(xorstr_("Vertical"), &m_vertical, 0.0f, 100.0f, xorstr_("%.0f%%"));
	ui::toggle(xorstr_("Jump Reset"), &m_jump_reset);
	if (m_jump_reset)
		ui::slider_float(xorstr_("Chance"), &m_jump_reset_chance, 0.0f, 100.0f, xorstr_("%.0f%%"));
}
