#include "../headers/nofall.h"

void c_nofall::on_tick(std::shared_ptr<c_context> ctx)
{

	if (ctx->local->get_object())
		ctx->local->set_fall_distance(0.0f);
}
