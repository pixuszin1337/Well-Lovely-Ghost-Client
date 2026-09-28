#pragma once

#include "impl.h"
#include "../../module.h"

class c_nickhider : public c_module {
	static inline c_nickhider* s_instance = nullptr;

	char m_fake_name[32] = "WellLovely";
	std::string m_original_name;
	bool m_entity_applied = false;
	bool m_tab_applied = false;
	std::string m_status;

	static inline jfieldID s_profile_fid = nullptr;
	static inline jfieldID s_name_fid = nullptr;
	static inline jfieldID s_sq_fid = nullptr;
	static inline jmethodID s_gpi_mid = nullptr;
	static inline jfieldID s_npi_gp_fid = nullptr;

	std::string read_gp_name(JNIEnv* env, jobject gp)
	{
		if (!s_name_fid || !gp) return {};
		auto js = (jstring)env->GetObjectField(gp, s_name_fid);
		if (!js) return {};
		const char* c = env->GetStringUTFChars(js, nullptr);
		std::string r = c ? c : "";
		if (c) env->ReleaseStringUTFChars(js, c);
		env->DeleteLocalRef(js);
		return r;
	}

	bool write_gp_name(JNIEnv* env, jobject gp, const char* name)
	{
		if (!s_name_fid || !gp) return false;
		auto js = env->NewStringUTF(name);
		if (!js) return false;
		env->SetObjectField(gp, s_name_fid, js);
		env->DeleteLocalRef(js);
		return true;
	}

	void resolve_entity(JNIEnv* env, jobject player)
	{
		if (s_profile_fid && s_name_fid)
			return;

		static auto last = std::chrono::steady_clock::time_point{};
		auto now = std::chrono::steady_clock::now();
		if (last.time_since_epoch().count() > 0 && now - last < std::chrono::seconds(3))
			return;
		last = now;

		extern jvmtiEnv* jvmti_env;
		auto cls = env->GetObjectClass(player);

		if (!s_profile_fid)
		{
			s_profile_fid = env->GetFieldID(cls,
				xorstr_("gameProfile"),
				xorstr_("Lcom/mojang/authlib/GameProfile;"));
			if (!s_profile_fid)
			{
				env->ExceptionClear();
				s_profile_fid = env->GetFieldID(cls,
					xorstr_("field_146106_i"),
					xorstr_("Lcom/mojang/authlib/GameProfile;"));
			}
			if (!s_profile_fid)
			{
				env->ExceptionClear();
				jclass cur = (jclass)env->NewLocalRef(cls);
				while (cur && !s_profile_fid)
				{
					jint fc = 0; jfieldID* flds = nullptr;
					jvmti_env->GetClassFields(cur, &fc, &flds);
					for (jint i = 0; i < fc && !s_profile_fid; i++)
					{
						char* sig = nullptr;
						jvmti_env->GetFieldName(cur, flds[i], nullptr, &sig, nullptr);
						if (sig && strstr(sig, "GameProfile"))
							s_profile_fid = flds[i];
						if (sig) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
					}
					if (flds) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(flds));
					jclass p = env->GetSuperclass(cur);
					env->DeleteLocalRef(cur);
					cur = p;
				}
				if (cur) env->DeleteLocalRef(cur);
			}
		}

		if (s_profile_fid && !s_name_fid)
		{
			auto gp = env->GetObjectField(player, s_profile_fid);
			if (gp)
			{
				auto pcls = env->GetObjectClass(gp);
				s_name_fid = env->GetFieldID(pcls,
					xorstr_("name"), xorstr_("Ljava/lang/String;"));
				if (!s_name_fid)
				{
					env->ExceptionClear();
					jint fc = 0; jfieldID* flds = nullptr;
					jvmti_env->GetClassFields(pcls, &fc, &flds);
					for (jint i = 0; i < fc && !s_name_fid; i++)
					{
						char* sig = nullptr;
						jvmti_env->GetFieldName(pcls, flds[i], nullptr, &sig, nullptr);
						if (sig && strcmp(sig, "Ljava/lang/String;") == 0)
							s_name_fid = flds[i];
						if (sig) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
					}
					if (flds) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(flds));
				}
				env->DeleteLocalRef(pcls);
				env->DeleteLocalRef(gp);
			}
		}

		env->DeleteLocalRef(cls);
	}

	void resolve_tab(JNIEnv* env, jobject player)
	{
		if (s_sq_fid && s_gpi_mid && s_npi_gp_fid)
			return;

		static auto last = std::chrono::steady_clock::time_point{};
		auto now = std::chrono::steady_clock::now();
		if (last.time_since_epoch().count() > 0 && now - last < std::chrono::seconds(3))
			return;
		last = now;

		extern jvmtiEnv* jvmti_env;

		if (!s_sq_fid)
		{
			auto cls = env->GetObjectClass(player);
			s_sq_fid = env->GetFieldID(cls,
				CUSTOM_CLIENT ? xorstr_("sendQueue") : BADLION_CLIENT ? xorstr_("g") : xorstr_("field_71174_a"),
				xorstr_("Lnet/minecraft/client/network/NetHandlerPlayClient;"));
			if (!s_sq_fid)
			{
				env->ExceptionClear();
				jint fc = 0; jfieldID* flds = nullptr;
				jvmti_env->GetClassFields(cls, &fc, &flds);
				for (jint i = 0; i < fc && !s_sq_fid; i++)
				{
					char* sig = nullptr;
					jvmti_env->GetFieldName(cls, flds[i], nullptr, &sig, nullptr);
					if (sig && strstr(sig, "NetHandler"))
						s_sq_fid = flds[i];
					if (sig) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
				}
				if (flds) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(flds));
			}
			env->DeleteLocalRef(cls);
		}

		if (!s_sq_fid) return;

		auto nh = env->GetObjectField(player, s_sq_fid);
		if (!nh) return;

		if (!s_gpi_mid)
		{
			auto nh_cls = env->GetObjectClass(nh);

			s_gpi_mid = env->GetMethodID(nh_cls,
				CUSTOM_CLIENT ? xorstr_("getPlayerInfo") : xorstr_("func_175104_a"),
				xorstr_("(Ljava/lang/String;)Lnet/minecraft/client/network/NetworkPlayerInfo;"));
			if (!s_gpi_mid)
			{
				env->ExceptionClear();
				jint mc = 0; jmethodID* mds = nullptr;
				jvmti_env->GetClassMethods(nh_cls, &mc, &mds);
				for (jint i = 0; i < mc && !s_gpi_mid; i++)
				{
					char* sig = nullptr;
					jvmti_env->GetMethodName(mds[i], nullptr, &sig, nullptr);
					if (sig && strstr(sig, "(Ljava/lang/String;)") && strstr(sig, "PlayerInfo"))
						s_gpi_mid = mds[i];
					if (sig) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
				}
				if (mds) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(mds));
			}

			env->DeleteLocalRef(nh_cls);
		}

		if (s_gpi_mid && !s_npi_gp_fid && !m_original_name.empty())
		{
			auto jname = env->NewStringUTF(m_original_name.c_str());
			if (jname)
			{
				auto info = env->CallObjectMethod(nh, s_gpi_mid, jname);
				env->DeleteLocalRef(jname);
				if (info)
				{
					auto info_cls = env->GetObjectClass(info);
					s_npi_gp_fid = env->GetFieldID(info_cls,
						xorstr_("gameProfile"),
						xorstr_("Lcom/mojang/authlib/GameProfile;"));
					if (!s_npi_gp_fid)
					{
						env->ExceptionClear();
						s_npi_gp_fid = env->GetFieldID(info_cls,
							xorstr_("field_178863_b"),
							xorstr_("Lcom/mojang/authlib/GameProfile;"));
					}
					if (!s_npi_gp_fid)
					{
						env->ExceptionClear();
						jint fc = 0; jfieldID* flds = nullptr;
						jvmti_env->GetClassFields(info_cls, &fc, &flds);
						for (jint i = 0; i < fc && !s_npi_gp_fid; i++)
						{
							char* sig = nullptr;
							jvmti_env->GetFieldName(info_cls, flds[i], nullptr, &sig, nullptr);
							if (sig && strstr(sig, "GameProfile"))
								s_npi_gp_fid = flds[i];
							if (sig) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
						}
						if (flds) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(flds));
					}
					env->DeleteLocalRef(info_cls);
					env->DeleteLocalRef(info);
				}
			}
		}

		env->DeleteLocalRef(nh);
	}

	void apply_entity_name(JNIEnv* env, jobject player)
	{
		if (!s_profile_fid || !s_name_fid)
			return;

		if (env->ExceptionCheck())
			env->ExceptionClear();

		auto gp = env->GetObjectField(player, s_profile_fid);
		if (!gp)
		{
			m_status = "GP object null";
			return;
		}

		std::string current;

		auto gp_cls = env->GetObjectClass(gp);
		auto gn_mid = env->GetMethodID(gp_cls,
			xorstr_("getName"), xorstr_("()Ljava/lang/String;"));
		if (!gn_mid) env->ExceptionClear();
		env->DeleteLocalRef(gp_cls);

		if (gn_mid)
		{
			auto jn = (jstring)env->CallObjectMethod(gp, gn_mid);
			if (jn)
			{
				const char* c = env->GetStringUTFChars(jn, nullptr);
				if (c) { current = c; env->ReleaseStringUTFChars(jn, c); }
				env->DeleteLocalRef(jn);
			}
		}

		if (current.empty())
			current = read_gp_name(env, gp);

		if (m_original_name.empty() && !current.empty())
			m_original_name = current;

		if (current.empty())
		{
			m_status = "GP name empty";
			env->DeleteLocalRef(gp);
			return;
		}

		if (current == std::string(m_fake_name))
		{
			m_entity_applied = true;
			env->DeleteLocalRef(gp);
			return;
		}

		auto fake = env->NewStringUTF(m_fake_name);
		if (fake)
		{
			env->SetObjectField(gp, s_name_fid, fake);
			env->DeleteLocalRef(fake);

			if (env->ExceptionCheck())
			{
				env->ExceptionClear();
				m_status = "SetField threw";
			}
			else
				m_entity_applied = true;
		}

		env->DeleteLocalRef(gp);
	}

	void apply_tab_name(JNIEnv* env, jobject player)
	{
		if (!s_sq_fid || !s_gpi_mid || !s_npi_gp_fid || !s_name_fid)
			return;
		if (m_original_name.empty())
			return;

		static auto last_check = std::chrono::steady_clock::time_point{};
		auto now = std::chrono::steady_clock::now();
		if (m_tab_applied && now - last_check < std::chrono::milliseconds(500))
			return;
		last_check = now;

		auto nh = env->GetObjectField(player, s_sq_fid);
		if (!nh) return;

		auto jname = env->NewStringUTF(m_original_name.c_str());
		if (!jname) { env->DeleteLocalRef(nh); return; }

		auto info = env->CallObjectMethod(nh, s_gpi_mid, jname);
		env->DeleteLocalRef(jname);
		env->DeleteLocalRef(nh);

		if (!info && m_tab_applied)
		{
			nh = env->GetObjectField(player, s_sq_fid);
			if (nh)
			{
				jname = env->NewStringUTF(m_fake_name);
				if (jname)
				{
					info = env->CallObjectMethod(nh, s_gpi_mid, jname);
					env->DeleteLocalRef(jname);
				}
				env->DeleteLocalRef(nh);
			}
		}
		if (!info) return;

		auto npi_gp = env->GetObjectField(info, s_npi_gp_fid);
		env->DeleteLocalRef(info);
		if (!npi_gp) return;

		auto current = read_gp_name(env, npi_gp);
		if (current != std::string(m_fake_name))
		{
			write_gp_name(env, npi_gp, m_fake_name);
			m_tab_applied = true;
		}

		env->DeleteLocalRef(npi_gp);
	}

	void restore_all(JNIEnv* env, jobject player)
	{
		if (m_original_name.empty())
			return;

		if (m_entity_applied && s_profile_fid && s_name_fid)
		{
			auto gp = env->GetObjectField(player, s_profile_fid);
			if (gp)
			{
				write_gp_name(env, gp, m_original_name.c_str());
				env->DeleteLocalRef(gp);
			}
			m_entity_applied = false;
		}

		if (m_tab_applied && s_sq_fid && s_gpi_mid && s_npi_gp_fid && s_name_fid)
		{
			auto nh = env->GetObjectField(player, s_sq_fid);
			if (nh)
			{
				auto jname = env->NewStringUTF(m_fake_name);
				if (jname)
				{
					auto info = env->CallObjectMethod(nh, s_gpi_mid, jname);
					env->DeleteLocalRef(jname);
					if (!info)
					{
						jname = env->NewStringUTF(m_original_name.c_str());
						if (jname)
						{
							info = env->CallObjectMethod(nh, s_gpi_mid, jname);
							env->DeleteLocalRef(jname);
						}
					}
					if (info)
					{
						auto npi_gp = env->GetObjectField(info, s_npi_gp_fid);
						if (npi_gp)
						{
							write_gp_name(env, npi_gp, m_original_name.c_str());
							env->DeleteLocalRef(npi_gp);
						}
						env->DeleteLocalRef(info);
					}
				}
				env->DeleteLocalRef(nh);
			}
			m_tab_applied = false;
		}
	}

public:
	c_nickhider() : c_module(xorstr_("Nick Hider"), e_category::render)
	{
		s_instance = this;
	}

	static std::string get_display_name(const std::string& real)
	{
		if (!s_instance || !s_instance->enabled)
			return real;
		if (!s_instance->m_original_name.empty() && real == s_instance->m_original_name)
			return std::string(s_instance->m_fake_name);
		return real;
	}

	void on_tick(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx->local || !ctx->local->get_object())
			return;

		auto env = welllovely::instance->get_env();
		if (!env)
			return;

		if (env->ExceptionCheck())
			env->ExceptionClear();

		auto player = ctx->local->get_object();

		resolve_entity(env, player);
		apply_entity_name(env, player);
		resolve_tab(env, player);
		apply_tab_name(env, player);

		if (!s_profile_fid)
			m_status = "Profile FID null";
		else if (!s_name_fid)
			m_status = "Name FID null";
		else if (m_entity_applied)
		{
			m_status = "Active";
			if (m_tab_applied) m_status += " + Tab";
			else if (!s_gpi_mid) m_status += " (tab: resolving)";
		}
	}

	void on_disable(std::shared_ptr<c_context> ctx) override
	{
		if (!ctx || !ctx->local || !ctx->local->get_object())
			return;

		auto env = welllovely::instance->get_env();
		if (!env)
			return;

		restore_all(env, ctx->local->get_object());
		m_original_name.clear();
		m_status.clear();
	}

	void on_render() override
	{
		ImGui::InputText(xorstr_("Fake Name"), m_fake_name, sizeof(m_fake_name));

		if (!m_status.empty())
		{
			ImGui::Dummy(ImVec2(1, 4));
			bool ok = m_status.find("Active") != std::string::npos;
			ImGui::TextColored(
				ok ? ImVec4(0.4f, 1.f, 0.4f, 1.f) : ImVec4(1.f, 0.7f, 0.3f, 1.f),
				"%s", m_status.c_str());
			if (!m_original_name.empty())
			{
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.575f, 0.553f, 0.616f, 1.f),
					"(%s)", m_original_name.c_str());
			}
		}
	}

	void save_config(config_data& d) override
	{
		d.set_str(xorstr_("nh_name"), m_fake_name);
	}

	void load_config(const config_data& d) override
	{
		auto name = d.get_str(xorstr_("nh_name"), xorstr_("WellLovely"));
		strncpy_s(m_fake_name, sizeof(m_fake_name), name.c_str(), _TRUNCATE);
	}
};
