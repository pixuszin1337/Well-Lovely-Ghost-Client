#include "../headers/strafe.h"

static float get_direction(std::shared_ptr<c_context> ctx);
static float get_speed(std::shared_ptr<c_context> ctx);
static void  set_speed(std::shared_ptr<c_context> ctx, const float speed);

void c_strafe::on_tick(std::shared_ptr<c_context> ctx)
{

	const auto is_sane = [&]() {
		return (ctx->local->get_object() && ctx->world->get_object());
	};

	if (!is_sane())
		return;

	if (!m_timer.has_passed(50))
		return;

	if (ctx->local->get_hurt_time() == 0
		&& (ctx->local->get_moveforward() != .0f
			|| ctx->local->get_movestrafe() != .0f))
		set_speed(ctx, get_speed(ctx));

	m_timer.reset();
}

static float get_direction(std::shared_ptr<c_context> ctx)
{
	float yaw = ctx->local->get_yaw();
	float move_forward = ctx->local->get_moveforward();
	float move_strafe = ctx->local->get_movestrafe();

	if (move_forward < 0.0f)
		yaw += 180.0f;

	float forward;

	if (move_forward < 0.0f)
		forward = -.5f;
	else if (move_forward > 0.0f)
		forward = .5f;
	else
		forward = 1.0f;

	if (move_strafe > 0.0f)
		yaw -= 90.0f * forward;
	else if (move_strafe < 0.0f)
		yaw += 90.0f * forward;

	yaw *= .017453292f;
	return yaw;
}

static float get_speed(std::shared_ptr<c_context> ctx)
{
	auto velocity_vector = ctx->local->get_motion_vector();

	return sqrt(velocity_vector.x * velocity_vector.x + velocity_vector.z * velocity_vector.z);
}

static void set_speed(std::shared_ptr<c_context> ctx, const float speed)
{
	auto direction = get_direction(ctx);
	auto velocity_vector = ctx->local->get_motion_vector();

	velocity_vector.x = -(sin(direction) * speed);
	velocity_vector.z = cos(direction) * speed;

	ctx->local->get_motion_vector(velocity_vector);
}
