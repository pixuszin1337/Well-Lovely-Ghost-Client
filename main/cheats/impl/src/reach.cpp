#include "../headers/reach.h"

void c_reach::on_tick(std::shared_ptr<c_context> ctx)
{
	const auto is_sane = [&]() {
		return (ctx->local->get_object() && ctx->world->get_object());
	};

	if (is_sane() && m_timer.has_passed(25))
	{
		float total;
		std::uniform_real_distribution<float> chance(0.0f, 100.0f);
		if (m_rare_chance > 0.0f && chance(m_rng) < m_rare_chance)
		{
			total = m_rare_dist;
		}
		else
		{
			float lo = m_min_reach, hi = m_max_reach;
			if (lo > hi) std::swap(lo, hi);
			std::uniform_real_distribution<float> rdist(lo, hi);
			total = rdist(m_rng);
		}

		auto distance = total - auth::decode_float(m_reach_base, auth::session_key());
		if (distance < 0.0f) distance = 0.0f;

		auto position = ctx->local->get_position();

		for (const auto& entity : ctx->world->get_players())
		{
			if (welllovely::instance->get_env()->IsSameObject(ctx->local->get_object(), entity->get_object()))
				continue;

			if (entity->get_health() <= 0.0f)
				continue;

			auto entity_position = entity->get_position();

			auto hypothenuse_distance = hypot(position.x - entity_position.x, position.z - entity_position.z);

			if (distance > hypothenuse_distance)
				distance -= hypothenuse_distance;

			auto angles = sdk::util::get_angles(position, entity_position);

			auto difference = sdk::util::wrap_to_180(-(ctx->local->get_yaw() - angles.first));

			if (std::abs(difference) > 180.0f)
				continue;

			auto cos = std::cos(sdk::util::deg_to_radiants(angles.first + 90.0f));
			auto sin = std::sin(sdk::util::deg_to_radiants(angles.first + 90.0f));

			auto x = entity_position.x, z = entity_position.z;
			x -= (cos * distance);
			z -= (sin * distance);

			auto entity_width = 0.6f;
			auto bb_width = entity_width / 2.0f;

			s_axisalignedbb bb{};
			bb.minX = x - bb_width;
			bb.minZ = z - bb_width;
			bb.maxX = x + bb_width;
			bb.maxZ = z + bb_width;

			if (sdk::version == sdk::e_version::v1_8_9)
			{

				bb.minY = entity_position.y;
				bb.maxY = entity_position.y + 1.8;
				entity->set_bounding_box(bb);
			}
			else
			{

				auto current_boundingbox = entity->get_bounding_box()->get_native_boundingbox();
				bb.minY = current_boundingbox.minY;
				bb.maxY = current_boundingbox.maxY;
				entity->get_bounding_box()->set_native_boundingbox(bb);
			}
		}
	}
}

void c_reach::on_render()
{
	if (ui::slider_float(xorstr_("Min reach"), &m_min_reach, 3.0f, 6.0f, xorstr_("%.2f")))
		if (m_min_reach > m_max_reach) m_max_reach = m_min_reach;
	if (ui::slider_float(xorstr_("Max reach"), &m_max_reach, 3.0f, 6.0f, xorstr_("%.2f")))
		if (m_max_reach < m_min_reach) m_min_reach = m_max_reach;
	ui::slider_float(xorstr_("Rare chance"), &m_rare_chance, 0.0f, 100.0f, xorstr_("%.0f%%"));
	ui::slider_float(xorstr_("Rare dist"), &m_rare_dist, 3.0f, 6.0f, xorstr_("%.2f"));
}
