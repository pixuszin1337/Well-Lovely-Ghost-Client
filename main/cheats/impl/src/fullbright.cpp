#include "../headers/fullbright.h"

void c_fullbright::on_tick(std::shared_ptr<c_context> ctx)
{

	if (!m_saved)
	{
		m_original = sdk::instance->get_gamma(ctx->minecraft);
		m_saved = true;
	}

	sdk::instance->set_gamma(ctx->minecraft, 100.0f);
}

void c_fullbright::on_disable(std::shared_ptr<c_context> ctx)
{

	if (m_saved)
	{
		sdk::instance->set_gamma(ctx->minecraft, m_original);
		m_saved = false;
	}
}
