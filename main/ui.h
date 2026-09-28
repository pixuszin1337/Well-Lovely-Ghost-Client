#pragma once

#include "../sdk/includes.h"

namespace ui {

	constexpr int accent_r = 243, accent_g = 139, accent_b = 170;

	inline bool slider_float(const char* label, float* v, float mn, float mx, const char* fmt = "%.1f")
	{
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::SameLine();
		ImGui::PushItemWidth(-1.0f);
		const std::string id = std::string("##") + label;
		const bool changed = ImGui::SliderFloat(id.c_str(), v, mn, mx, fmt);
		ImGui::PopItemWidth();
		return changed;
	}

	inline bool slider_int(const char* label, int* v, int mn, int mx, const char* fmt = "%d")
	{
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::SameLine();
		ImGui::PushItemWidth(-1.0f);
		const std::string id = std::string("##") + label;
		const bool changed = ImGui::SliderInt(id.c_str(), v, mn, mx, fmt);
		ImGui::PopItemWidth();
		return changed;
	}

	inline bool toggle(const char* label, bool* v)
	{
		const float line = ImGui::GetFrameHeight();
		const float h = line * 0.72f;
		const float w = h * 1.85f;
		const float r = h * 0.5f;

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);

		ImGui::SameLine();
		const float avail = ImGui::GetContentRegionAvail().x;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail > w ? avail - w : 0.f));

		const ImVec2 p = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton(label, ImVec2(w, line));
		const bool clicked = ImGui::IsItemClicked();
		if (clicked)
			*v = !*v;
		const bool hovered = ImGui::IsItemHovered();

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float y = p.y + (line - h) * 0.5f;

		const int g = hovered ? 74 : 60;
		const ImU32 col_off = IM_COL32(g, g, g + 12, 255);
		const ImU32 col_on = IM_COL32(accent_r, accent_g, accent_b, 255);

		dl->AddRectFilled(ImVec2(p.x, y), ImVec2(p.x + w, y + h), *v ? col_on : col_off, r);
		const float kx = *v ? (p.x + w - r) : (p.x + r);
		dl->AddCircleFilled(ImVec2(kx, y + r), r - 2.0f, IM_COL32(255, 255, 255, 255));

		return clicked;
	}
}
