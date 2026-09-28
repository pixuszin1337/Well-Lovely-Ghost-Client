#pragma once

#include "../../module.h"
#include <unordered_set>
#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>

class c_friends : public c_module {
	char m_name_buf[64] = {};

	static std::string get_file_path()
	{
		char appdata[MAX_PATH]{};
		if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) == 0)
			return "";
		auto dir = std::filesystem::path(appdata) / "WellLovely";
		std::filesystem::create_directories(dir);
		return (dir / "friends.txt").string();
	}

	static std::string to_lower(const std::string& s)
	{
		std::string out = s;
		std::transform(out.begin(), out.end(), out.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return out;
	}

public:
	static inline std::unordered_set<std::string> list;

	c_friends() : c_module("Friends", e_category::utils) { show_toggle = false; load_friends(); }

	static bool is_friend(const std::string& name)
	{
		return list.count(to_lower(name)) > 0;
	}

	static void add(const std::string& name)
	{
		if (!name.empty())
		{
			list.insert(to_lower(name));
			save_friends();
		}
	}

	static void remove(const std::string& name)
	{
		list.erase(to_lower(name));
		save_friends();
	}

	static void save_friends()
	{
		auto path = get_file_path();
		if (path.empty()) return;
		std::ofstream f(path);
		if (!f) return;
		for (const auto& name : list)
			f << name << "\n";
	}

	static void load_friends()
	{
		auto path = get_file_path();
		if (path.empty()) return;
		std::ifstream f(path);
		if (!f) return;
		list.clear();
		std::string line;
		while (std::getline(f, line))
		{
			if (!line.empty())
				list.insert(to_lower(line));
		}
	}

	void on_render() override
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 120);
		ImGui::InputText("##friendname", m_name_buf, sizeof(m_name_buf));
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 8);
		if (ImGui::Button("Add", ImVec2(100, 0)) && m_name_buf[0])
		{
			add(m_name_buf);
			m_name_buf[0] = '\0';
		}

		ImGui::Dummy(ImVec2(1, 8));
		ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 1.0f), "Friends List");
		ImGui::Dummy(ImVec2(1, 4));

		if (list.empty())
			ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 0.6f), "No friends added yet.");

		std::string to_remove;
		for (const auto& name : list)
		{
			ImGui::PushID(name.c_str());
			ImGui::TextColored(ImVec4(0.953f, 0.941f, 0.953f, 1.0f), "%s", name.c_str());
			ImGui::SameLine(0, 16);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.12f, 0.12f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.15f, 0.15f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
			if (ImGui::Button("Remove", ImVec2(60, 0)))
				to_remove = name;
			ImGui::PopStyleColor(3);
			ImGui::PopID();
			ImGui::Dummy(ImVec2(1, 2));
		}

		if (!to_remove.empty())
			remove(to_remove);
	}
};
