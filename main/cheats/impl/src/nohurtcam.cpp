#include "../headers/nohurtcam.h"

void c_nohurtcam::on_tick(std::shared_ptr<c_context> ctx)
{
	if (ctx->local->get_object())
		ctx->local->set_hurt_time(0);
}
