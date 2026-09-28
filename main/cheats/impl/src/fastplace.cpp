#include "../headers/fastplace.h"

void c_fastplace::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->minecraft)
		return;

	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(ctx->minecraft);

	auto fid = env->GetFieldID(cls,
		CUSTOM_CLIENT ? xorstr_("rightClickDelayTimer") : BADLION_CLIENT ? xorstr_("ah") : xorstr_("field_71467_ac"),
		xorstr_("I"));

	env->DeleteLocalRef(cls);

	if (!fid)
	{
		env->ExceptionClear();
		return;
	}

	int cur = env->GetIntField(ctx->minecraft, fid);
	if (cur > m_delay_ticks)
		env->SetIntField(ctx->minecraft, fid, m_delay_ticks);
}

void c_fastplace::on_render()
{
	ImGui::SliderInt(xorstr_("Delay (ticks)"), &m_delay_ticks, 0, 4);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(xorstr_("0 = instant, 4 = vanilla"));
}

void c_fastplace::save_config(config_data& data)
{
	data.set_int(xorstr_("fastplace_delay"), m_delay_ticks);
}

void c_fastplace::load_config(const config_data& data)
{
	m_delay_ticks = data.get_int(xorstr_("fastplace_delay"), 0);
}
