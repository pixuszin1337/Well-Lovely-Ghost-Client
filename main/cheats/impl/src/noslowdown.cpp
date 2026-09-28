#include "../headers/noslowdown.h"
#include <cmath>

void c_noslowdown::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->local->get_object())
		return;

	auto env = welllovely::instance->get_env();

	if (!m_resolved)
	{
		m_resolved = true;
		auto cls = env->GetObjectClass(ctx->local->get_object());

		m_fwd_fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("moveForward") : BADLION_CLIENT ? xorstr_("ba") : xorstr_("field_70701_bs"),
			xorstr_("F"));
		if (!m_fwd_fid) env->ExceptionClear();

		m_str_fid = env->GetFieldID(cls,
			CUSTOM_CLIENT ? xorstr_("moveStrafing") : BADLION_CLIENT ? xorstr_("aZ") : xorstr_("field_70702_br"),
			xorstr_("F"));
		if (!m_str_fid) env->ExceptionClear();

		env->DeleteLocalRef(cls);
	}

	if (!m_fwd_fid || !m_str_fid)
		return;

	float fwd = env->GetFloatField(ctx->local->get_object(), m_fwd_fid);
	float strafe = env->GetFloatField(ctx->local->get_object(), m_str_fid);

	bool slowed = (fwd != 0.0f && std::abs(fwd) < 0.21f) ||
	              (strafe != 0.0f && std::abs(strafe) < 0.21f);

	if (!slowed)
	{
		m_did_write = false;
		return;
	}

	auto motion = ctx->local->get_motion_vector();

	if (m_did_write && motion.x == m_written_x && motion.z == m_written_z)
		return;

	float full_fwd = fwd * 5.0f;
	float full_str = strafe * 5.0f;

	float yaw_rad = ctx->local->get_yaw() * 3.14159265f / 180.0f;
	float sy = std::sin(yaw_rad);
	float cy = std::cos(yaw_rad);

	float move_speed = ctx->local->get_ai_move_speed();
	if (move_speed < 0.0f) move_speed = 0.1f;

	if (ctx->local->is_sprinting())
		move_speed /= 1.3f;

	double drag;
	double friction_slowed;
	double friction_full;

	if (ctx->local->is_on_ground())
	{
		drag = 0.6 * 0.91;
		double factor = 0.16277136 / (drag * drag * drag);
		friction_slowed = move_speed * factor;
		friction_full = move_speed * 1.3 * factor;
	}
	else
	{
		drag = 0.91;
		friction_slowed = 0.02;
		friction_full = 0.02 + 0.006;
	}

	float sd = std::sqrt(fwd * fwd + strafe * strafe);
	if (sd < 1.0f) sd = 1.0f;
	double sf = friction_slowed / sd;
	double slowed_dx = (strafe * cy - fwd * sy) * sf;
	double slowed_dz = (fwd * cy + strafe * sy) * sf;

	float fd = std::sqrt(full_fwd * full_fwd + full_str * full_str);
	if (fd < 1.0f) fd = 1.0f;
	double ff = friction_full / fd;
	double full_dx = (full_str * cy - full_fwd * sy) * ff;
	double full_dz = (full_fwd * cy + full_str * sy) * ff;

	motion.x += full_dx - slowed_dx;
	motion.z += full_dz - slowed_dz;

	ctx->local->get_motion_vector(motion);
	m_written_x = motion.x;
	m_written_z = motion.z;
	m_did_write = true;

	if (full_fwd > 0.0f)
		ctx->local->set_sprinting(true);
}
