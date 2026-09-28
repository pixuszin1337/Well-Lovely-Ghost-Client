#pragma once

#include "../../module.h"
#include "../../config.h"

class c_configs : public c_module {
	char m_name_buf[64] = {};
public:
	c_configs() : c_module("Configs", e_category::misc) { show_toggle = false; }

	void on_render() override
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 120);
		ImGui::InputText("##cfgname", m_name_buf, sizeof(m_name_buf));
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 8);
		if (ImGui::Button("Save", ImVec2(100, 0)) && m_name_buf[0]) {
			config::save(m_name_buf);
			config::save_last(m_name_buf);
		}

		ImGui::Dummy(ImVec2(1, 8));
		ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 1.0f), "Saved Configs");
		ImGui::Dummy(ImVec2(1, 4));

		auto cfgs = config::list();
		if (cfgs.empty())
			ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 0.6f), "No configs saved yet.");

		for (const auto& cfg : cfgs) {
			ImGui::PushID(cfg.c_str());
			ImGui::TextColored(ImVec4(0.953f, 0.941f, 0.953f, 1.0f), "%s", cfg.c_str());
			ImGui::SameLine(0, 16);
			if (ImGui::Button("Load", ImVec2(60, 0))) {
				config::load(cfg);
				config::save_last(cfg);
			}
			ImGui::SameLine(0, 8);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.12f, 0.12f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.15f, 0.15f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
			if (ImGui::Button("Delete", ImVec2(60, 0)))
				config::remove(cfg);
			ImGui::PopStyleColor(3);
			ImGui::PopID();
			ImGui::Dummy(ImVec2(1, 2));
		}
	}
};
