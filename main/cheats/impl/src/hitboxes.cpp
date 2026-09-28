#include "../headers/hitboxes.h"

void c_hitboxes::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->local->get_object() || !ctx->world->get_object())
		return;

	if (!m_timer.has_passed(50))
		return;

	for (auto& entity : ctx->world->get_players())
	{
		if (welllovely::instance->get_env()->IsSameObject(ctx->local->get_object(), entity->get_object()))
			continue;
		if (entity->get_health() <= 0.0f)
			continue;

		auto pos = entity->get_position();
		const double s = m_size;

		s_axisalignedbb bb{};
		bb.minX = pos.x - 0.3 - s; bb.maxX = pos.x + 0.3 + s;
		bb.minZ = pos.z - 0.3 - s; bb.maxZ = pos.z + 0.3 + s;
		bb.minY = pos.y - s;       bb.maxY = pos.y + 1.8 + s;

		if (sdk::version == sdk::e_version::v1_8_9)
			entity->set_bounding_box(bb);
		else
			entity->get_bounding_box()->set_native_boundingbox(bb);
	}

	m_timer.reset();
}

void c_hitboxes::on_render()
{
	ui::slider_float(xorstr_("Size"), &m_size, 0.1f, 1.0f, xorstr_("%.2f"));
}
