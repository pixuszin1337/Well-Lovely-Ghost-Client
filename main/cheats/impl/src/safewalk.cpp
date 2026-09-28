#include "../headers/safewalk.h"
#include "../../../../sdk/minecraft/axisaligned/axisalignedbb.h"
#include <cmath>

namespace
{

	bool has_support_any(std::shared_ptr<c_context> ctx, const s_axisalignedbb& bb, double dx, double dz, int max_drop)
	{
		int y_top = (int)std::ceil(bb.minY) - 1;
		if (y_top < 0)
			return true;

		int y_bottom = y_top - max_drop;
		if (y_bottom < 0)
			y_bottom = 0;

		double x0 = bb.minX + dx, x1 = bb.maxX + dx;
		double z0 = bb.minZ + dz, z1 = bb.maxZ + dz;

		int bx0 = (int)std::floor(x0), bx1 = (int)std::floor(x1);
		int bz0 = (int)std::floor(z0), bz1 = (int)std::floor(z1);

		for (int y = y_top; y >= y_bottom; y--)
		{
			for (int bx = bx0; bx <= bx1; bx++)
			{
				if (!(x0 < (double)bx + 1.0 && x1 > (double)bx))
					continue;
				for (int bz = bz0; bz <= bz1; bz++)
				{
					if (!(z0 < (double)bz + 1.0 && z1 > (double)bz))
						continue;
					if (!ctx->world->is_air_block(bx, y, bz))
						return true;
				}
			}
		}

		return false;
	}

	bool has_support(std::shared_ptr<c_context> ctx, const s_axisalignedbb& bb, double dx, double dz, double margin, int max_drop)
	{
		if (!has_support_any(ctx, bb, dx, dz, max_drop))
			return false;

		if (margin <= 0.0)
			return true;

		return has_support_any(ctx, bb, dx + margin, dz + margin, max_drop)
			&& has_support_any(ctx, bb, dx + margin, dz - margin, max_drop)
			&& has_support_any(ctx, bb, dx - margin, dz + margin, max_drop)
			&& has_support_any(ctx, bb, dx - margin, dz - margin, max_drop);
	}

	double shrink_step(double v)
	{
		if (v > -0.05 && v < 0.05)
			return 0.0;
		return v > 0.0 ? v - 0.05 : v + 0.05;
	}
}

void c_safewalk::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->minecraft || !ctx->local->get_object() || !ctx->ingame || !ctx->world)
		return;

	if (!ctx->local->is_on_ground())
	{
		m_did_write = false;
		return;
	}

	auto motion = ctx->local->get_motion_vector();

	if (m_did_write
		&& std::abs(motion.x - m_written_x) < 1.0e-9
		&& std::abs(motion.z - m_written_z) < 1.0e-9)
		return;

	m_did_write = false;

	auto bb = ctx->local->get_bounding_box()->get_native_boundingbox();
	auto yaw = ctx->local->get_yaw();

	float forward = 0.0f, strafe = 0.0f;
	if (!sdk::instance->get_move_input(ctx->minecraft, forward, strafe))
	{
		forward = ctx->local->get_moveforward();
		strafe = ctx->local->get_movestrafe();
	}

	double speed = ctx->local->get_ai_move_speed();
	if (speed <= 0.0)
		speed = ctx->local->is_sprinting() ? 0.13 : 0.1;

	auto now = std::chrono::steady_clock::now();
	unsigned phys =
		((GetAsyncKeyState('W') & 0x8000) ? 1u : 0u) |
		((GetAsyncKeyState('S') & 0x8000) ? 2u : 0u) |
		((GetAsyncKeyState('A') & 0x8000) ? 4u : 0u) |
		((GetAsyncKeyState('D') & 0x8000) ? 8u : 0u) |
		((GetAsyncKeyState(VK_SPACE) & 0x8000) ? 16u : 0u) |
		((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 32u : 0u);

	if (phys != m_last_keys)
	{
		m_last_keys = phys;
		m_last_change = now;
	}

	if (std::abs(yaw - m_last_yaw) > 0.01f)
		m_last_change = now;
	m_last_yaw = yaw;

	bool pending =
		((phys & 1u) != 0u) != (forward > 0.5f) ||
		((phys & 2u) != 0u) != (forward < -0.5f) ||
		((phys & 4u) != 0u) != (strafe > 0.5f) ||
		((phys & 8u) != 0u) != (strafe < -0.5f);

	bool hot = pending || (now - m_last_change) < std::chrono::milliseconds(60);

	double margin = hot ? speed + ((phys & 16u) ? 0.21 : 0.0) : 0.0;

	double fx = 0.0, fz = 0.0;
	double mag = (double)strafe * strafe + (double)forward * forward;
	if (mag >= 1.0e-4)
	{
		double r = std::sqrt(mag);
		if (r < 1.0)
			r = 1.0;
		double scale = speed / r;

		double s = strafe * scale;
		double fw = forward * scale;

		double rad = yaw * 3.14159265358979323846 / 180.0;
		double sy = std::sin(rad), cy = std::cos(rad);

		fx = s * cy - fw * sy;
		fz = fw * cy + s * sy;
	}

	double tx = motion.x + fx;
	double tz = motion.z + fz;

	double cx = tx, cz = tz;
	while (cx != 0.0 && !has_support(ctx, bb, cx, 0.0, margin, m_safe_drop))
		cx = shrink_step(cx);
	while (cz != 0.0 && !has_support(ctx, bb, cx, cz, margin, m_safe_drop))
		cz = shrink_step(cz);
	while (cx != 0.0 && !has_support(ctx, bb, cx, cz, margin, m_safe_drop))
		cx = shrink_step(cx);

	double nx = cx - fx;
	double nz = cz - fz;

	if (std::abs(nx - motion.x) > 1.0e-9 || std::abs(nz - motion.z) > 1.0e-9)
	{
		ctx->local->get_motion_vector(sdk::vec3d{ nx, motion.y, nz });
		m_written_x = nx;
		m_written_z = nz;
		m_did_write = true;
	}
}
