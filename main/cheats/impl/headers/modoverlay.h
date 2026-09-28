#pragma once

#include "impl.h"
#include "../../module_manager.h"
#include <algorithm>
#include <vector>

class c_modoverlay : public c_module {
public:
	c_modoverlay() : c_module(xorstr_("Mod Overlay"), e_category::render) {}

	void on_draw(ImDrawList* draw) override
	{
		if (!modules::instance)
			return;

		if (welllovely::instance->b_open)
			return;

		std::vector<const std::string*> names;
		for (auto& m : modules::instance->get())
		{
			if (!m->enabled || !m->show_toggle)
				continue;
			if (m.get() == this)
				continue;
			names.push_back(&m->name);
		}

		if (names.empty())
			return;

		std::sort(names.begin(), names.end(),
			[](const std::string* a, const std::string* b) { return *a < *b; });

		const float pad = 8.0f;
		const float line_h = ImGui::GetTextLineHeightWithSpacing();

		float max_w = 0.0f;
		for (auto* n : names)
			max_w = (std::max)(max_w, ImGui::CalcTextSize(n->c_str()).x);

		auto& io = ImGui::GetIO();
		const float x1 = io.DisplaySize.x - 12.0f;
		const float y0 = 12.0f;
		const float w = max_w + pad * 2.0f + 10.0f;

		draw->AddRectFilled(
			ImVec2(x1 - w, y0),
			ImVec2(x1, y0 + line_h * (float)names.size() + pad * 2.0f),
			IM_COL32(18, 15, 22, 160), 6.0f);

		float y = y0 + pad;
		for (auto* n : names)
		{
			draw->AddCircleFilled(ImVec2(x1 - w + pad + 2.0f, y + line_h * 0.5f - 2.0f), 2.5f, IM_COL32(243, 139, 170, 255));
			draw->AddText(ImVec2(x1 - w + pad + 10.0f, y), IM_COL32(235, 235, 240, 255), n->c_str());
			y += line_h;
		}
	}
};
