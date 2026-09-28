#include "../headers/step.h"

static void set_step_height(jobject player_obj, float value)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto fid = env->GetFieldID(cls,
		CUSTOM_CLIENT ? xorstr_("stepHeight") : BADLION_CLIENT ? xorstr_("U") : xorstr_("field_70138_W"),
		xorstr_("F"));

	env->DeleteLocalRef(cls);

	if (!fid)
	{
		env->ExceptionClear();
		return;
	}

	env->SetFloatField(player_obj, fid, value);
}

void c_step::on_tick(std::shared_ptr<c_context> ctx)
{
	if (ctx->local->get_object())
		set_step_height(ctx->local->get_object(), m_height);
}

void c_step::on_disable(std::shared_ptr<c_context> ctx)
{
	if (ctx->local->get_object())
		set_step_height(ctx->local->get_object(), 0.6f);
}

void c_step::on_render()
{
	ImGui::SliderFloat(xorstr_("Height"), &m_height, 0.6f, 2.5f, "%.1f");
}

void c_step::save_config(config_data& data)
{
	data.set_float(xorstr_("step_height"), m_height);
}

void c_step::load_config(const config_data& data)
{
	m_height = data.get_float(xorstr_("step_height"), 1.0f);
}
