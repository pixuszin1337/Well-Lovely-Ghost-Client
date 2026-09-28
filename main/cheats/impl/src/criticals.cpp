#include "../headers/criticals.h"

void c_criticals::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->local->get_object())
		return;

	bool lmb = GetAsyncKeyState(VK_LBUTTON) & 0x8000;

	if (!lmb)
	{
		m_was_clicking = false;
		return;
	}

	if (m_was_clicking)
		return;

	m_was_clicking = true;

	if (!ctx->local->is_on_ground())
		return;

	auto motion = ctx->local->get_motion_vector();
	motion.y = 0.1;
	ctx->local->get_motion_vector(motion);
	ctx->local->set_on_ground(false);
}
