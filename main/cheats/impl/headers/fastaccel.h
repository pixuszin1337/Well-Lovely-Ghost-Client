#pragma once

#include "impl.h"
#include "../../module.h"

class c_fastaccel : public c_module {
	float m_boost = 50.0f;

public:
	c_fastaccel() : c_module(xorstr_("Fast Accel"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->local->get_object()) return;
		if (!ctx->local->is_on_ground()) return;

		bool w = GetAsyncKeyState('W') & 0x8000;
		bool a = GetAsyncKeyState('A') & 0x8000;
		bool s = GetAsyncKeyState('S') & 0x8000;
		bool d = GetAsyncKeyState('D') & 0x8000;

		if (!w && !a && !s && !d) return;

		auto motion = ctx->local->get_motion_vector();
		double current_speed = std::sqrt(motion.x * motion.x + motion.z * motion.z);

		float move_speed = ctx->local->get_ai_move_speed();
		if (move_speed < 0.0f) move_speed = 0.1f;

		double slipperiness = 0.6;
		double drag = slipperiness * 0.91;
		double accel_factor = 0.16277136 / (drag * drag * drag);
		double acceleration = move_speed * accel_factor;
		double max_speed = acceleration * drag / (1.0 - drag);

		if (max_speed <= 0.0 || current_speed >= max_speed) return;

		double boost_factor = m_boost / 100.0;

		if (current_speed < 0.001)
		{
			float yaw = ctx->local->get_yaw();
			double dir = yaw * 3.141592653589793 / 180.0;

			float fwd = 0.0f, stf = 0.0f;
			if (w) fwd += 1.0f; if (s) fwd -= 1.0f;
			if (a) stf += 1.0f; if (d) stf -= 1.0f;

			double angle = std::atan2(fwd, stf) - 1.5707963267948966;
			dir += angle;

			double target_speed = max_speed * boost_factor;
			motion.x = -std::sin(dir) * target_speed;
			motion.z = std::cos(dir) * target_speed;
		}
		else
		{
			double target_speed = current_speed + (max_speed - current_speed) * boost_factor;
			double scale = target_speed / current_speed;
			motion.x *= scale;
			motion.z *= scale;
		}

		ctx->local->get_motion_vector(motion);
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Boost %"), &m_boost, 10.0f, 100.0f, xorstr_("%.0f%%"));
	}

	void save_config(config_data& d) override { d.set_float(xorstr_("fa_boost"), m_boost); }
	void load_config(const config_data& d) override { m_boost = d.get_float(xorstr_("fa_boost"), 50.0f); }
};
