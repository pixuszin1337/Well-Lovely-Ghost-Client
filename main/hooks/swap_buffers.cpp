#include "../welllovely.h"
#include "veh_hook.h"

#include "../cheats/module_manager.h"
#include "../cheats/config.h"
#include "../cheats/impl/headers/notifications.h"

#include "../welllovely_logo.h"

#include "../inter_font.h"
#include "../lovedays_font.h"

#include "../icons.h"

std::shared_ptr<wrapper::c_context> welllovely::hooks::gl_context = nullptr;

static GLuint g_logo_tex = 0;

static ImFont* g_font_lovedays = nullptr;

static GLuint g_icon_tex[7] = { 0, 0, 0, 0, 0, 0, 0 };

static const unsigned char* icon_for(e_category c)
{
	switch (c)
	{
	case e_category::combat:   return icon_combat;
	case e_category::movement: return icon_movement;
	case e_category::render:   return icon_render;
	case e_category::utils:    return icon_misc;
	case e_category::block:    return icon_block;
	default:                   return nullptr;
	}
}

static GLuint ensure_icon(e_category c)
{
	const int i = static_cast<int>(c);
	if (i < 0 || i >= 7)
		return 0;
	if (g_icon_tex[i] == 0)
	{
		const unsigned char* data = icon_for(c);
		if (!data)
			return 0;
		glGenTextures(1, &g_icon_tex[i]);
		glBindTexture(GL_TEXTURE_2D, g_icon_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, icon_w, icon_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	return g_icon_tex[i];
}

void set_colors();

static void setup_overlay(HWND hwnd, HDC hdc)
{
	static bool created = false;

	if (created)
	{
		ImGui_ImplOpenGL2_Shutdown();
		ImGui_ImplWin32_Shutdown();
	}

	if (!welllovely::hooks::gl_context)
		welllovely::hooks::gl_context = std::make_shared<wrapper::c_context>();
	if (welllovely::hooks::gl_context->m_glrenderctx)
		wglDeleteContext(welllovely::hooks::gl_context->m_glrenderctx);
	welllovely::hooks::gl_context->m_hdc_devicectx = hdc;
	welllovely::hooks::gl_context->m_glrenderctx = wglCreateContext(hdc);

	if (!created)
	{
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		ImFontConfig font_cfg;
		font_cfg.FontDataOwnedByAtlas = false;

		ImFont* inter = io.Fonts->AddFontFromMemoryTTF((void*)inter_ttf, (int)inter_ttf_len, 16.0f, &font_cfg);
		if (inter)
			io.FontDefault = inter;
		else
			io.Fonts->AddFontDefault();

		font_cfg.FontDataOwnedByAtlas = false;
		g_font_lovedays = io.Fonts->AddFontFromMemoryTTF((void*)lovedays_ttf, (int)lovedays_ttf_len, 22.0f, &font_cfg);

		set_colors();
		created = true;
	}

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplOpenGL2_Init();

	auto prev = reinterpret_cast<WNDPROC>(
		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(welllovely::hooks::wndproc_hk)));

	if (prev != reinterpret_cast<WNDPROC>(welllovely::hooks::wndproc_hk))
		welllovely::hooks::original_wndproc = prev;

	welllovely::hooks::mc_hwnd = hwnd;

	g_logo_tex = 0;
	for (int i = 0; i < 7; ++i)
		g_icon_tex[i] = 0;
}

static int call_original_swap(HDC hdc)
{
	if (CUSTOM_CLIENT)
	{
		veh::in_original = true;
		int ret = reinterpret_cast<welllovely::hooks::swap_buffers_fn>(veh::target_addr)(hdc);
		veh::in_original = false;
		return ret;
	}
	else
	{
		return welllovely::hooks::oswap_buffers(hdc);
	}
}

int __stdcall welllovely::hooks::swap_buffers_hk(HDC hdc) {

	if (!welllovely::instance || !welllovely::instance->b_running)
		return call_original_swap(hdc);

	HWND cur_window = WindowFromDC(hdc);

	if (!cur_window || IsIconic(cur_window))
		return call_original_swap(hdc);

	if (GetForegroundWindow() != cur_window)
		return call_original_swap(hdc);

	if (cur_window != welllovely::hooks::mc_hwnd)
		setup_overlay(cur_window, hdc);

	if (!gl_context || !gl_context->m_glrenderctx)
		return call_original_swap(hdc);

	HGLRC game_ctx = wglGetCurrentContext();
	if (!game_ctx)
		return call_original_swap(hdc);

	if (!wglMakeCurrent(hdc, gl_context->m_glrenderctx))
		return call_original_swap(hdc);

	if (CUSTOM_CLIENT)
	{
		static jclass mouse_cls = nullptr;
		static jmethodID set_grabbed = nullptr;
		static jmethodID mouse_next_mid = nullptr;
		static jclass kb_cls = nullptr;
		static jmethodID kb_next_mid = nullptr;
		static jclass mc_cls = nullptr;
		static jfieldID the_mc_fid = nullptr;
		static jfieldID focus_fid = nullptr;
		static bool jni_cached = false;

		JNIEnv* jni = welllovely::instance->get_env();

		if (jni && !jni_cached)
		{
			jclass local;

			local = welllovely::instance->find_class_quiet("org.lwjgl.input.Mouse");
			if (local)
			{
				mouse_cls = (jclass)jni->NewGlobalRef(local);
				jni->DeleteLocalRef(local);
				set_grabbed = jni->GetStaticMethodID(mouse_cls, "setGrabbed", "(Z)V");
				mouse_next_mid = jni->GetStaticMethodID(mouse_cls, "next", "()Z");
			}

			local = welllovely::instance->find_class_quiet("org.lwjgl.input.Keyboard");
			if (local)
			{
				kb_cls = (jclass)jni->NewGlobalRef(local);
				jni->DeleteLocalRef(local);
				kb_next_mid = jni->GetStaticMethodID(kb_cls, "next", "()Z");
			}

			local = welllovely::instance->find_class_quiet("net.minecraft.client.Minecraft");
			if (local)
			{
				mc_cls = (jclass)jni->NewGlobalRef(local);
				jni->DeleteLocalRef(local);
				the_mc_fid = jni->GetStaticFieldID(mc_cls,
					xorstr_("theMinecraft"), xorstr_("Lnet/minecraft/client/Minecraft;"));
				focus_fid = jni->GetFieldID(mc_cls, xorstr_("inGameHasFocus"), xorstr_("Z"));
			}

			jni_cached = true;
		}

		static bool menu_was_open = false;
		if (welllovely::instance->b_open != menu_was_open)
		{
			menu_was_open = welllovely::instance->b_open;
			if (jni)
			{
				if (set_grabbed)
					jni->CallStaticVoidMethod(mouse_cls, set_grabbed, (jboolean)!menu_was_open);

				if (focus_fid && the_mc_fid)
				{
					jobject mc = jni->GetStaticObjectField(mc_cls, the_mc_fid);
					if (mc)
					{
						jni->SetBooleanField(mc, focus_fid, (jboolean)!menu_was_open);
						jni->DeleteLocalRef(mc);
					}
				}
			}
		}

		if (welllovely::instance->b_open && jni)
		{
			if (set_grabbed)
				jni->CallStaticVoidMethod(mouse_cls, set_grabbed, (jboolean)JNI_FALSE);
			if (focus_fid && the_mc_fid)
			{
				jobject mc = jni->GetStaticObjectField(mc_cls, the_mc_fid);
				if (mc)
				{
					jni->SetBooleanField(mc, focus_fid, (jboolean)JNI_FALSE);
					jni->DeleteLocalRef(mc);
				}
			}

			if (kb_next_mid)
				while (jni->CallStaticBooleanMethod(kb_cls, kb_next_mid)) {}
			if (mouse_next_mid)
				while (jni->CallStaticBooleanMethod(mouse_cls, mouse_next_mid)) {}
		}
	}

	ImGuiIO& io = ImGui::GetIO();
	if (CUSTOM_CLIENT)
		io.MouseDrawCursor = false;
	else
		io.MouseDrawCursor = welllovely::instance->b_open;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplWin32_NewFrame();

	if (CUSTOM_CLIENT)
	{
		io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
		io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
		io.MouseDown[3] = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
		io.MouseDown[4] = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
		for (int i = 0; i < 256; i++)
			io.KeysDown[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
	}

	ImGui::NewFrame();

	if (modules::instance)
	{
		ImDrawList* bg = ImGui::GetBackgroundDrawList();
		for (const auto& m : modules::instance->get())
			if (m->enabled)
				m->on_draw(bg);

		static std::unordered_map<c_module*, bool> notif_prev;
		for (const auto& m : modules::instance->get())
		{
			if (!m->show_toggle) continue;
			auto it = notif_prev.find(m.get());
			if (it != notif_prev.end() && it->second != m->enabled)
				c_notifications::push(m->name, m->enabled);
			notif_prev[m.get()] = m->enabled;
		}
	}

	if (welllovely::instance->b_open && modules::instance)
	{
		ImGuiIO& io_menu = ImGui::GetIO();
		ImGui::SetNextWindowPos(
			ImVec2(io_menu.DisplaySize.x * 0.5f - 440.f, io_menu.DisplaySize.y * 0.5f - 270.f),
			ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(880.f, 540.f), ImGuiCond_Once);
		ImGui::SetNextWindowSizeConstraints(ImVec2(880.f, 640.f), ImVec2(1280.f, 800.f));

		ImGui::Begin(xorstr_("##well_lovely"), nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

		{
			ImVec2 wpos = ImGui::GetWindowPos();
			ImVec2 wsz  = ImGui::GetWindowSize();
			ImDrawList* bg_dl = ImGui::GetBackgroundDrawList();
			for (int i = 1; i <= 10; i++)
			{
				float a = (10 - i) * 8.f;
				bg_dl->AddRect(
					ImVec2(wpos.x - i, wpos.y - i),
					ImVec2(wpos.x + wsz.x + i, wpos.y + wsz.y + i),
					IM_COL32(0, 0, 0, (int)a), 14.f + i, 0, 1.0f);
			}
		}

		static e_category selected = e_category::combat;
		static const e_category cats[] = {
			e_category::combat, e_category::movement, e_category::player,
			e_category::render, e_category::utils, e_category::block, e_category::misc
		};

		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(16, 16, 20, 255));
		ImGui::BeginChild(xorstr_("##sidebar"), ImVec2(248.f, 0.f), false);
		{
			ImGui::Dummy(ImVec2(1, 16));

			if (g_logo_tex == 0)
			{
				glGenTextures(1, &g_logo_tex);
				glBindTexture(GL_TEXTURE_2D, g_logo_tex);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, welllovely_w, welllovely_h, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, welllovely_rgba);
			}

			const float lw = 248.f;
			const float lh = lw * welllovely_h / welllovely_w;
			float sidebar_w = ImGui::GetContentRegionAvail().x;
			ImGui::SetCursorPosX((sidebar_w - lw) * 0.5f);
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(g_logo_tex)), ImVec2(lw, lh));

			{
				const char* sub = "Best Legit Client";

				ImVec2 sub_sz = ImGui::CalcTextSize(sub);
				ImGui::SetCursorPosX((sidebar_w - sub_sz.x) * 0.5f);
				ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 1.0f), "%s", sub);

			}

			ImGui::Dummy(ImVec2(1, 8));
			{
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				dl->AddLine(ImVec2(p.x + 16.f, p.y), ImVec2(p.x + sidebar_w - 16.f, p.y),
					IM_COL32(41, 38, 48, 255), 1.0f);
			}
			ImGui::Dummy(ImVec2(1, 16));

			for (auto cat : cats)
			{
				bool has_any = (cat == e_category::misc);
				if (!has_any) {
					for (const auto& m : modules::instance->get())
						if (m->category == cat) { has_any = true; break; }
				}
				if (!has_any)
					continue;

				bool active = selected == cat;
				const float rowH = 40.f;
				const float marginX = 16.f;
				const float btnW = sidebar_w - marginX * 2;

				ImGui::PushID(static_cast<int>(cat));
				ImGui::SetCursorPosX(marginX);
				ImVec2 bpos = ImGui::GetCursorScreenPos();

				if (ImGui::InvisibleButton("##cat", ImVec2(btnW, rowH)))
					selected = cat;

				ImDrawList* dl = ImGui::GetWindowDrawList();
				bool hovered = ImGui::IsItemHovered();

				ImVec2 rmin = bpos;
				ImVec2 rmax(bpos.x + btnW, bpos.y + rowH);

				if (active)
					dl->AddRectFilled(rmin, rmax, IM_COL32(243, 139, 170, 255), 8.f);
				else if (hovered)
					dl->AddRectFilled(rmin, rmax, IM_COL32(36, 34, 46, 255), 8.f);

				const float iconSz = 18.f;
				float textX = rmin.x + 14.f;
				GLuint tex = ensure_icon(cat);
				if (tex)
				{
					ImVec2 imin(rmin.x + 14.f, rmin.y + (rowH - iconSz) * 0.5f);
					ImVec2 imax(imin.x + iconSz, imin.y + iconSz);
					ImU32 tint = active ? IM_COL32(30, 30, 38, 255) : IM_COL32(210, 210, 220, 255);
					dl->AddImage(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(tex)),
						imin, imax, ImVec2(0, 0), ImVec2(1, 1), tint);
					textX = imax.x + 10.f;
				}

				const char* cname = category_name(cat);
				ImU32 tcol = active ? IM_COL32(30, 30, 38, 255) : IM_COL32(240, 240, 245, 255);
				ImVec2 tsz = ImGui::CalcTextSize(cname);
				dl->AddText(ImVec2(textX, rmin.y + (rowH - tsz.y) * 0.5f), tcol, cname);

				ImVec2 asz = ImGui::CalcTextSize(">");
				dl->AddText(ImVec2(rmax.x - 20.f, rmin.y + (rowH - asz.y) * 0.5f), tcol, ">");

				ImGui::PopID();
				ImGui::Dummy(ImVec2(1, 4));
			}

			ImGui::SetCursorPos(ImVec2(24, ImGui::GetWindowHeight() - 36));
			ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 1.0f), "Well Lovely");
			ImGui::SameLine(sidebar_w - 50);
			ImGui::TextColored(ImVec4(0.953f, 0.545f, 0.667f, 1.0f), "v1.0");
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();

		{
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImVec2 wpos = ImGui::GetWindowPos();
			ImVec2 wsz  = ImGui::GetWindowSize();
			dl->AddLine(ImVec2(wpos.x + 248.f, wpos.y), ImVec2(wpos.x + 248.f, wpos.y + wsz.y),
				IM_COL32(41, 38, 48, 255), 1.0f);
		}

		ImGui::SameLine(0, 0);

		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(24, 24, 30, 255));
		ImGui::BeginChild(xorstr_("##content"), ImVec2(0.f, 0.f), false);
		{
			ImGui::Dummy(ImVec2(1, 14));
			ImGui::SetCursorPosX(28);

			ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 1.0f), "%s  >", category_name(selected));
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.953f, 0.941f, 0.953f, 1.0f), "Modules");

			ImGui::Dummy(ImVec2(1, 6));
			{
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				float cw = ImGui::GetContentRegionAvail().x;
				dl->AddLine(ImVec2(p.x + 28.f, p.y), ImVec2(p.x + cw - 16.f, p.y),
					IM_COL32(41, 38, 48, 255), 1.0f);
			}
			ImGui::Dummy(ImVec2(1, 14));

			ImGui::SetCursorPosX(28);
			ImGui::TextColored(ImVec4(0.953f, 0.545f, 0.667f, 1.0f), "Configuration");
			ImGui::Dummy(ImVec2(1, 8));

			ImGui::SetCursorPosX(28);
			ImGui::BeginChild(xorstr_("##modlist"), ImVec2(ImGui::GetContentRegionAvail().x - 28, 0), false);
			{
				static c_module* expanded_mod = nullptr;

				for (const auto& m : modules::instance->get())
				{
					if (m->category != selected)
						continue;

					ImGui::PushID(m.get());

					bool is_exp = (expanded_mod == m.get());
					float line_h = ImGui::GetFrameHeight();
					ImVec2 row_start = ImGui::GetCursorScreenPos();
					float full_w = ImGui::GetContentRegionAvail().x;

					float th = line_h * 0.72f;
					float tw = th * 1.85f;
					float tr = th * 0.5f;

					float row_w = m->show_toggle ? (full_w - tw - 8) : full_w;
					if (ImGui::InvisibleButton("##row", ImVec2(row_w, line_h)))
						expanded_mod = is_exp ? nullptr : m.get();
					bool row_hov = ImGui::IsItemHovered();

					bool tog_hov = false;
					if (m->show_toggle)
					{
						ImGui::SameLine(0, 0);
						ImGui::InvisibleButton("##tog", ImVec2(tw + 8, line_h));
						if (ImGui::IsItemClicked())
							m->enabled = !m->enabled;
						tog_hov = ImGui::IsItemHovered();
					}

					ImDrawList* dl = ImGui::GetWindowDrawList();

					if (row_hov || tog_hov)
						dl->AddRectFilled(row_start, ImVec2(row_start.x + full_w, row_start.y + line_h),
							IM_COL32(36, 34, 46, 128), 4.f);

					float ty = row_start.y + (line_h - ImGui::CalcTextSize("A").y) * 0.5f;
					ImU32 name_col = is_exp ? IM_COL32(243, 139, 170, 255) : IM_COL32(240, 240, 245, 255);
					const char* arrow = is_exp ? "v" : ">";
					dl->AddText(ImVec2(row_start.x + 4, ty), IM_COL32(243, 139, 170, 255), arrow);
					dl->AddText(ImVec2(row_start.x + 24, ty), name_col, m->name.c_str());

					if (m->show_toggle)
					{
						float toggle_x = row_start.x + full_w - tw - 4;
						float toggle_y = row_start.y + (line_h - th) * 0.5f;

						int g = tog_hov ? 74 : 60;
						ImU32 col_off = IM_COL32(g, g, g + 12, 255);
						ImU32 col_on = IM_COL32(ui::accent_r, ui::accent_g, ui::accent_b, 255);

						dl->AddRectFilled(ImVec2(toggle_x, toggle_y), ImVec2(toggle_x + tw, toggle_y + th),
							m->enabled ? col_on : col_off, tr);
						float kx = m->enabled ? (toggle_x + tw - tr) : (toggle_x + tr);
						dl->AddCircleFilled(ImVec2(kx, toggle_y + tr), tr - 2.0f, IM_COL32(255, 255, 255, 255));
					}

					if (is_exp)
					{
						ImGui::Indent(24.f);
						ImGui::PushItemWidth(-64.f);

						if (m->show_toggle)
						{
							bool this_binding = modules::instance->is_binding()
								&& modules::instance->binding() == m.get();
							std::string keylabel = this_binding ? std::string("...") : vk_to_string(m->keybind);
							std::string btn_id = std::string(xorstr_("Bind: ")) + keylabel + "##bind";
							if (ImGui::SmallButton(btn_id.c_str()))
								modules::instance->begin_bind(m.get());
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(xorstr_("Clique e aperte uma tecla (Esc = remover)"));
						}

						m->on_render();

						ImGui::PopItemWidth();
						ImGui::Unindent(24.f);
					}

					ImGui::Spacing();
					{
						ImVec2 p = ImGui::GetCursorScreenPos();
						float w = ImGui::GetContentRegionAvail().x;
						dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + w - 8, p.y),
							IM_COL32(41, 38, 48, 180), 1.0f);
					}
					ImGui::Dummy(ImVec2(1, 4));

					ImGui::PopID();
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	wglMakeCurrent(hdc, game_ctx);

	return call_original_swap(hdc);
}

void set_colors()
{
	const ImVec4 pink      = ImVec4(0.953f, 0.545f, 0.667f, 1.0f);
	const ImVec4 pink_dark = ImVec4(0.725f, 0.255f, 0.439f, 1.0f);
	const ImVec4 bg_main   = ImVec4(0.047f, 0.047f, 0.059f, 1.0f);
	const ImVec4 panel     = ImVec4(0.090f, 0.090f, 0.110f, 1.0f);
	const ImVec4 panel2    = ImVec4(0.118f, 0.110f, 0.137f, 1.0f);
	const ImVec4 text_main = ImVec4(0.953f, 0.941f, 0.953f, 1.0f);
	const ImVec4 text_dim  = ImVec4(0.575f, 0.553f, 0.616f, 1.0f);
	const ImVec4 border_c  = ImVec4(0.161f, 0.149f, 0.188f, 1.0f);

	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha             = 1.0f;
	style.WindowPadding     = ImVec2(0, 0);
	style.WindowMinSize     = ImVec2(32, 32);
	style.WindowRounding    = 14.0f;
	style.WindowBorderSize  = 1.0f;
	style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
	style.ChildRounding     = 10.0f;
	style.PopupRounding     = 8.0f;
	style.FramePadding      = ImVec2(12, 9);
	style.FrameRounding     = 8.0f;
	style.FrameBorderSize   = 0.0f;
	style.ItemSpacing       = ImVec2(10, 12);
	style.ItemInnerSpacing  = ImVec2(8, 6);
	style.IndentSpacing     = 20.0f;
	style.ScrollbarSize     = 11.0f;
	style.ScrollbarRounding = 8.0f;
	style.GrabMinSize       = 8.0f;
	style.GrabRounding      = 8.0f;
	style.TabRounding       = 8.0f;
	style.ButtonTextAlign   = ImVec2(0.5f, 0.5f);
	style.AntiAliasedLines  = true;

	ImVec4* c = style.Colors;
	c[ImGuiCol_Text]                 = text_main;
	c[ImGuiCol_TextDisabled]         = text_dim;
	c[ImGuiCol_WindowBg]             = bg_main;
	c[ImGuiCol_ChildBg]              = ImVec4(0, 0, 0, 0);
	c[ImGuiCol_PopupBg]              = panel2;
	c[ImGuiCol_Border]               = border_c;
	c[ImGuiCol_BorderShadow]         = ImVec4(0, 0, 0, 0);
	c[ImGuiCol_FrameBg]              = panel2;
	c[ImGuiCol_FrameBgHovered]       = ImVec4(0.161f, 0.149f, 0.188f, 1.0f);
	c[ImGuiCol_FrameBgActive]        = ImVec4(0.200f, 0.165f, 0.200f, 1.0f);
	c[ImGuiCol_TitleBg]              = bg_main;
	c[ImGuiCol_TitleBgActive]        = pink_dark;
	c[ImGuiCol_TitleBgCollapsed]     = bg_main;
	c[ImGuiCol_MenuBarBg]            = bg_main;
	c[ImGuiCol_ScrollbarBg]          = ImVec4(0, 0, 0, 0);
	c[ImGuiCol_ScrollbarGrab]        = border_c;
	c[ImGuiCol_ScrollbarGrabHovered] = pink_dark;
	c[ImGuiCol_ScrollbarGrabActive]  = pink;
	c[ImGuiCol_CheckMark]            = pink;
	c[ImGuiCol_SliderGrab]           = pink;
	c[ImGuiCol_SliderGrabActive]     = pink_dark;
	c[ImGuiCol_Button]               = panel2;
	c[ImGuiCol_ButtonHovered]        = ImVec4(0.220f, 0.165f, 0.200f, 1.0f);
	c[ImGuiCol_ButtonActive]         = pink_dark;
	c[ImGuiCol_Header]               = panel2;
	c[ImGuiCol_HeaderHovered]        = ImVec4(0.220f, 0.165f, 0.200f, 1.0f);
	c[ImGuiCol_HeaderActive]         = pink;
	c[ImGuiCol_Separator]            = border_c;
	c[ImGuiCol_SeparatorHovered]     = pink_dark;
	c[ImGuiCol_SeparatorActive]      = pink;
	c[ImGuiCol_ResizeGrip]           = border_c;
	c[ImGuiCol_ResizeGripHovered]    = pink_dark;
	c[ImGuiCol_ResizeGripActive]     = pink;
	c[ImGuiCol_Tab]                  = panel;
	c[ImGuiCol_TabHovered]           = pink_dark;
	c[ImGuiCol_TabActive]            = pink_dark;
	c[ImGuiCol_TabUnfocused]         = panel;
	c[ImGuiCol_TabUnfocusedActive]   = pink_dark;
	c[ImGuiCol_PlotLines]            = pink;
	c[ImGuiCol_PlotHistogram]        = pink;
	c[ImGuiCol_TextSelectedBg]       = ImVec4(pink.x, pink.y, pink.z, 0.35f);
	c[ImGuiCol_ModalWindowDarkening] = ImVec4(0, 0, 0, 0.55f);
}
