#pragma once

#include "../../module.h"
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

class c_notifications : public c_module {
	struct notification {
		std::string text;
		bool on;
		std::chrono::steady_clock::time_point time;
	};

	static inline std::vector<notification> m_queue;

	float m_duration = 2.0f;

public:
	c_notifications() : c_module(xorstr_("Notifications"), e_category::utils) { enabled = true; }

	static void push(const std::string& module_name, bool on)
	{
		m_queue.push_back({ module_name, on, std::chrono::steady_clock::now() });
	}

	void on_draw(ImDrawList* draw) override
	{
		auto now = std::chrono::steady_clock::now();

		m_queue.erase(std::remove_if(m_queue.begin(), m_queue.end(), [&](const notification& n) {
			return std::chrono::duration<float>(now - n.time).count() > m_duration;
		}), m_queue.end());

		if (m_queue.empty())
			return;

		ImGuiIO& io = ImGui::GetIO();
		float y = io.DisplaySize.y - 40.0f;

		for (int i = static_cast<int>(m_queue.size()) - 1; i >= 0; --i)
		{
			auto& n = m_queue[i];
			float age = std::chrono::duration<float>(now - n.time).count();

			float alpha = 1.0f;
			float fade_in = 0.15f;
			float fade_out = 0.4f;
			if (age < fade_in)
				alpha = age / fade_in;
			else if (age > m_duration - fade_out)
				alpha = (m_duration - age) / fade_out;
			if (alpha < 0.0f) alpha = 0.0f;
			if (alpha > 1.0f) alpha = 1.0f;

			std::string text = n.text + (n.on ? "  ON" : "  OFF");
			ImVec2 sz = ImGui::CalcTextSize(text.c_str());

			float pad_x = 12.0f;
			float pad_y = 8.0f;
			float bar_w = 3.0f;
			float box_w = sz.x + pad_x * 2 + bar_w;
			float box_h = sz.y + pad_y * 2;

			float bx = io.DisplaySize.x - box_w - 20.0f;
			float by = y - box_h;

			float slide = (1.0f - alpha) * 20.0f;
			bx += slide;

			draw->AddRectFilled(
				ImVec2(bx, by), ImVec2(bx + box_w, by + box_h),
				IM_COL32(16, 16, 20, static_cast<int>(200 * alpha)), 6.0f);

			draw->AddRectFilled(
				ImVec2(bx, by + 2.0f), ImVec2(bx + bar_w, by + box_h - 2.0f),
				n.on ? IM_COL32(100, 255, 100, static_cast<int>(255 * alpha))
				     : IM_COL32(255, 100, 100, static_cast<int>(255 * alpha)),
				2.0f);

			ImU32 text_col = IM_COL32(240, 240, 245, static_cast<int>(255 * alpha));
			draw->AddText(ImVec2(bx + bar_w + pad_x, by + pad_y), text_col, text.c_str());

			y -= box_h + 4.0f;
		}
	}

	void on_render() override
	{
		ui::slider_float(xorstr_("Duration"), &m_duration, 1.0f, 5.0f, xorstr_("%.1fs"));
	}

	void save_config(config_data& d) override
	{
		d.set_float(xorstr_("notif_duration"), m_duration);
	}

	void load_config(const config_data& d) override
	{
		m_duration = d.get_float(xorstr_("notif_duration"), 2.0f);
	}
};
