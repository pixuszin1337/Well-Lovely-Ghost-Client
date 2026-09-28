#include "../headers/sprint.h"

void c_sprint::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->local->get_object() || !ctx->ingame)
		return;

	if (ctx->local->get_moveforward() > 0.0f)
		ctx->local->set_sprinting(true);
}
