#include "welllovely.h"
#include "hooks/veh_hook.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include "../sdk/minecraft/minecraft.h"
#include "../sdk/minecraft/player/player.h"
#include "../sdk/minecraft/world/world.h"
#include "../sdk/input/input_hook.h"

#include "cheats/cheat.h"
#include "cheats/module_manager.h"
#include "cheats/config.h"

#include "cheats/impl/headers/aimbot.h"
#include "cheats/impl/headers/reach.h"
#include "cheats/impl/headers/strafe.h"
#include "cheats/impl/headers/autoclicker.h"
#include "cheats/impl/headers/sprint.h"
#include "cheats/impl/headers/esp.h"
#include "cheats/impl/headers/tracers.h"
#include "cheats/impl/headers/nametags.h"
#include "cheats/impl/headers/velocity.h"
#include "cheats/impl/headers/fullbright.h"
#include "cheats/impl/headers/nofall.h"
#include "cheats/impl/headers/hitboxes.h"
#include "cheats/impl/headers/configs.h"
#include "cheats/impl/headers/fastplace.h"
#include "cheats/impl/headers/nohurtcam.h"
#include "cheats/impl/headers/step.h"
#include "cheats/impl/headers/triggerbot.h"
#include "cheats/impl/headers/safewalk.h"
#include "cheats/impl/headers/noslowdown.h"
#include "cheats/impl/headers/criticals.h"
#include "cheats/impl/headers/wtap.h"
#include "cheats/impl/headers/invwalk.h"
#include "cheats/impl/headers/friends.h"
#include "cheats/impl/headers/notifications.h"
#include "cheats/impl/headers/nohitdelay.h"
#include "cheats/impl/headers/keepsprint.h"
#include "cheats/impl/headers/nojumpdelay.h"
#include "cheats/impl/headers/nousedelay.h"
#include "cheats/impl/headers/instantstop.h"
#include "cheats/impl/headers/timer.h"
#include "cheats/impl/headers/fastaccel.h"
#include "cheats/impl/headers/antidebuff.h"
#include "cheats/impl/headers/nullmove.h"
#include "cheats/impl/headers/sprintreset.h"
#include "cheats/impl/headers/fastmine.h"
#include "cheats/impl/headers/autoweapon.h"
#include "cheats/impl/headers/modoverlay.h"
#include "cheats/impl/headers/indicators.h"
#include "cheats/impl/headers/pointers.h"
#include "cheats/impl/headers/teams.h"
#include "cheats/impl/headers/antibot.h"
#include "cheats/impl/headers/autotool.h"
#include "cheats/impl/headers/autosoup.h"
#include "cheats/impl/headers/refill.h"
#include "cheats/impl/headers/autopot.h"
#include "cheats/impl/headers/chestesp.h"
#include "cheats/impl/headers/itemesp.h"
#include "cheats/impl/headers/trajectories.h"
#include "cheats/impl/headers/clutch.h"
#include "cheats/impl/headers/bridgeassist.h"
#include "cheats/impl/headers/blockin.h"
#include "cheats/impl/headers/nickhider.h"

jvmtiEnv* jvmti_env;

bool welllovely::c_welllovely::attach()
{
	auto jvm_dll = wrapper::get_module_handle(xorstr_("jvm.dll"));

	auto created_java_vms = reinterpret_cast<sdk::t_createdvms>(wrapper::get_proc_address(xorstr_("JNI_GetCreatedJavaVMs"), jvm_dll));

	auto ret = created_java_vms(&vm, 1, nullptr);

	if (ret != JNI_OK)
		return false;

	ret = get_vm()->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);

	if (ret != JNI_OK)
		return false;

	get_vm()->GetEnv(reinterpret_cast<void**>(&jvmti_env), JVMTI_VERSION_1_1);

	if (!jvmti_env)
		return false;

	{
		jint class_count = 0;
		jclass* classes = nullptr;
		jvmti_env->GetLoadedClasses(&class_count, &classes);

		bool found_custom = false;
		for (jint i = 0; i < class_count; i++)
		{
			char* sig = nullptr;
			jvmti_env->GetClassSignature(classes[i], &sig, nullptr);
			if (sig)
			{
				if (std::strstr(sig, xorstr_("com/moonsworth/")))
				{
					found_custom = true;
					jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
					break;
				}
				jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
			}
		}
		jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(classes));

		CUSTOM_CLIENT = found_custom;
	}

	get_launchwrapper();

	sdk::detect_version();

	hook();

	input_hook::start();

	b_running = true;

	return true;
}

void welllovely::c_welllovely::run()
{
	sdk::instance = std::make_unique<sdk::c_minecraft>();

	modules::instance = std::make_unique<modules::c_manager>();

	modules::instance->add<c_reach>();
	modules::instance->add<c_aimbot>();
	modules::instance->add<c_autoclicker>();
	modules::instance->add<c_strafe>();
	modules::instance->add<c_sprint>();
	modules::instance->add<c_esp>();
	modules::instance->add<c_tracers>();
	modules::instance->add<c_nametags>();
	modules::instance->add<c_nickhider>();
	modules::instance->add<c_velocity>();
	modules::instance->add<c_hitboxes>();
	modules::instance->add<c_fullbright>();
	modules::instance->add<c_nofall>();
	modules::instance->add<c_triggerbot>();
	modules::instance->add<c_fastplace>();
	modules::instance->add<c_nohurtcam>();
	modules::instance->add<c_step>();
	modules::instance->add<c_safewalk>();
	modules::instance->add<c_noslowdown>();
	modules::instance->add<c_criticals>();
	modules::instance->add<c_nohitdelay>();
	modules::instance->add<c_keepsprint>();
	modules::instance->add<c_wtap>();
	modules::instance->add<c_invwalk>();
	modules::instance->add<c_nojumpdelay>();
	modules::instance->add<c_instantstop>();
	modules::instance->add<c_nousedelay>();
	modules::instance->add<c_gamespeed>();
	modules::instance->add<c_fastaccel>();
	modules::instance->add<c_friends>();
	modules::instance->add<c_notifications>();
	modules::instance->add<c_antidebuff>();
	modules::instance->add<c_nullmove>();
	modules::instance->add<c_sprintreset>();
	modules::instance->add<c_fastmine>();
	modules::instance->add<c_autoweapon>();
	modules::instance->add<c_modoverlay>();
	modules::instance->add<c_indicators>();
	modules::instance->add<c_pointers>();
	modules::instance->add<c_teams>();
	modules::instance->add<c_antibot>();
	modules::instance->add<c_autotool>();
	modules::instance->add<c_autosoup>();
	modules::instance->add<c_refill>();
	modules::instance->add<c_autopot>();
	modules::instance->add<c_chestesp>();
	modules::instance->add<c_itemesp>();
	modules::instance->add<c_trajectories>();
	modules::instance->add<c_clutch>();
	modules::instance->add<c_bridgeassist>();
	modules::instance->add<c_blockin>();
	modules::instance->add<c_configs>();

	config::load_last();

	while (b_running)
	{
		if (wrapper::get_async_keystate(VK_HOME))
			b_running = false;

		auto tick_env = get_env();
		if (!tick_env || tick_env->PushLocalFrame(512) < 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}

		if (tick_env->ExceptionCheck())
			tick_env->ExceptionClear();

		{
			auto minecraft_inst = sdk::instance->get_minecraft();

			if (minecraft_inst)
			{
				auto local = std::make_shared<c_player>(sdk::instance->get_player(minecraft_inst));
				auto world = std::make_shared<c_world>(sdk::instance->get_world(minecraft_inst));

				auto screen = sdk::instance->get_current_screen(minecraft_inst);
				bool ingame = !screen;
				if (screen) tick_env->DeleteLocalRef(screen);

				modules::instance->on_tick(std::make_shared<c_context>(local, world, ingame, false, minecraft_inst));

				tick_env->DeleteLocalRef(minecraft_inst);
			}
		}

		tick_env->PopLocalFrame(nullptr);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void welllovely::c_welllovely::dispose()
{
	input_hook::stop();

	if (!SKIP_RENDER_HOOKS)
	{
		if (hooks::mc_hwnd && hooks::original_wndproc)
			SetWindowLongPtrA(hooks::mc_hwnd, GWLP_WNDPROC, (LONG_PTR)hooks::original_wndproc);

		if (CUSTOM_CLIENT)
		{
			veh::remove();
		}
		else
		{
			MH_DisableHook(MH_ALL_HOOKS);
			MH_Uninitialize();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(250));

		ImGui_ImplOpenGL2_Shutdown();
		if (hooks::gl_context && hooks::gl_context->m_glrenderctx)
			wglDeleteContext(hooks::gl_context->m_glrenderctx);
		ImGui::DestroyContext();
		ImGui_ImplWin32_Shutdown();
	}

	get_env()->DeleteGlobalRef(classloader_obj);
	get_vm()->DetachCurrentThread();

	env = nullptr;
	hooks::gl_context = nullptr;
	vm = nullptr;
}

static void dump_vanilla_classes()
{
	char appdata[MAX_PATH]{};
	if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) == 0)
		return;

	auto dir = std::filesystem::path(appdata) / "WellLovely";
	std::filesystem::create_directories(dir);
	auto path = dir / "vanilla_dump.txt";

	std::ofstream f(path);
	if (!f) return;

	f << "=====================================\n";
	f << " VANILLA DUMP - WellLovely\n";
	f << " Versao: " << sdk::version_string() << "\n";
	f << "=====================================\n\n";

	auto env = welllovely::instance->get_env();

	struct class_info { const char* notch; const char* mcp; };
	class_info to_dump[] = {
		{ "ave",  "Minecraft" },
		{ "pk",   "Entity" },
		{ "bew",  "EntityPlayerSP" },
		{ "bdb",  "WorldClient" },
		{ "aug",  "AxisAlignedBB" },
		{ "auh",  "MovingObjectPosition" },
		{ "axu",  "GuiScreen" },
		{ "avl",  "Timer" },
		{ "avh",  "GameSettings" },
	};

	for (auto& info : to_dump)
	{
		auto cls = welllovely::instance->find_class_quiet(info.notch);
		if (!cls)
		{
			f << "=== " << info.notch << " (" << info.mcp << ") === NOT FOUND\n\n";
			continue;
		}

		f << "=== " << info.notch << " (" << info.mcp << ") ===\n";

		jint field_count = 0;
		jfieldID* fields = nullptr;
		jvmti_env->GetClassFields(cls, &field_count, &fields);

		f << "  FIELDS (" << field_count << "):\n";
		for (jint i = 0; i < field_count; i++)
		{
			char* name = nullptr;
			char* sig = nullptr;
			jvmti_env->GetFieldName(cls, fields[i], &name, &sig, nullptr);

			jint mods = 0;
			jvmti_env->GetFieldModifiers(cls, fields[i], &mods);

			f << "    " << (name ? name : "?") << " " << (sig ? sig : "?");
			if (mods & 0x0008) f << " [static]";
			f << "\n";

			if (name) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(name));
			if (sig)  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
		}
		if (fields) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(fields));

		jint method_count = 0;
		jmethodID* methods = nullptr;
		jvmti_env->GetClassMethods(cls, &method_count, &methods);

		f << "  METHODS (" << method_count << "):\n";
		for (jint i = 0; i < method_count; i++)
		{
			char* name = nullptr;
			char* sig = nullptr;
			jvmti_env->GetMethodName(methods[i], &name, &sig, nullptr);

			f << "    " << (name ? name : "?") << " " << (sig ? sig : "?") << "\n";

			if (name) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(name));
			if (sig)  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
		}
		if (methods) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(methods));

		f << "\n";
		env->DeleteLocalRef(cls);
	}

	f << "\n=====================================\n";
	f << " SCAN: ActiveRenderInfo + RenderManager\n";
	f << "=====================================\n";
	f << "Procurando classes com static FloatBuffer (ARI) e static double renderPos (RM)...\n\n";

	jint scan_count = 0;
	jclass* scan_classes = nullptr;
	jvmti_env->GetLoadedClasses(&scan_count, &scan_classes);

	for (jint ci = 0; ci < scan_count; ci++)
	{
		char* cls_sig = nullptr;
		jvmti_env->GetClassSignature(scan_classes[ci], &cls_sig, nullptr);
		if (!cls_sig) continue;

		std::string sig_str(cls_sig);
		jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(cls_sig));

		if (sig_str.size() < 2 || sig_str[0] != 'L') continue;
		if (sig_str.find('/') != std::string::npos) continue;

		jint fc = 0;
		jfieldID* flds = nullptr;
		jvmti_env->GetClassFields(scan_classes[ci], &fc, &flds);
		if (fc == 0) { if (flds) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(flds)); continue; }

		int static_floatbuf = 0;
		int static_intbuf = 0;
		int static_double = 0;

		for (jint fi = 0; fi < fc; fi++)
		{
			char* fn = nullptr; char* fs = nullptr;
			jvmti_env->GetFieldName(scan_classes[ci], flds[fi], &fn, &fs, nullptr);
			jint fm = 0;
			jvmti_env->GetFieldModifiers(scan_classes[ci], flds[fi], &fm);

			if (fm & 0x0008)
			{
				if (fs && std::strcmp(fs, "Ljava/nio/FloatBuffer;") == 0) static_floatbuf++;
				if (fs && std::strcmp(fs, "Ljava/nio/IntBuffer;") == 0)   static_intbuf++;
				if (fs && std::strcmp(fs, "D") == 0)                      static_double++;
			}

			if (fn) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(fn));
			if (fs) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(fs));
		}

		bool is_ari = (static_floatbuf >= 2 && static_intbuf >= 1);
		bool is_rm  = (static_double >= 3 && fc >= 5 && fc <= 30);

		if (is_ari || is_rm)
		{
			std::string cls_name = sig_str.substr(1, sig_str.size() - 2);
			f << "=== " << cls_name;
			if (is_ari) f << " (ActiveRenderInfo?)";
			if (is_rm)  f << " (RenderManager?)";
			f << " ===\n";

			f << "  FIELDS (" << fc << "):\n";
			for (jint fi = 0; fi < fc; fi++)
			{
				char* fn = nullptr; char* fs = nullptr;
				jvmti_env->GetFieldName(scan_classes[ci], flds[fi], &fn, &fs, nullptr);
				jint fm = 0;
				jvmti_env->GetFieldModifiers(scan_classes[ci], flds[fi], &fm);

				f << "    " << (fn ? fn : "?") << " " << (fs ? fs : "?");
				if (fm & 0x0008) f << " [static]";
				f << "\n";

				if (fn) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(fn));
				if (fs) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(fs));
			}
			f << "\n";
		}

		if (flds) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(flds));
	}

	if (scan_classes) jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(scan_classes));

	f << "Dump completo. Use os nomes acima para preencher os '?' no codigo.\n";
	f.close();

	wrapper::dbg_log(std::string("Vanilla dump: ") + path.string());
}

void welllovely::c_welllovely::get_launchwrapper()
{
	if (CUSTOM_CLIENT)
	{
		jint class_count = 0;
		jclass* classes = nullptr;
		jvmti_env->GetLoadedClasses(&class_count, &classes);

		jclass target_cls = nullptr;
		for (jint i = 0; i < class_count; i++)
		{
			char* sig = nullptr;
			jvmti_env->GetClassSignature(classes[i], &sig, nullptr);
			if (sig && std::strcmp(sig, xorstr_("Lnet/minecraft/client/Minecraft;")) == 0)
			{
				target_cls = classes[i];
				jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
				break;
			}
			if (sig)
				jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
		}

		if (!target_cls)
		{
			jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(classes));
			wrapper::dbg_log(xorstr_("Minecraft class not found via JVMTI"));
			std::exit(0);
		}

		jclass cls_class = get_env()->FindClass(xorstr_("java/lang/Class"));
		jmethodID get_cl_mid = get_env()->GetMethodID(cls_class, xorstr_("getClassLoader"), xorstr_("()Ljava/lang/ClassLoader;"));
		jobject cl = get_env()->CallObjectMethod(target_cls, get_cl_mid);
		get_env()->DeleteLocalRef(cls_class);

		if (!cl)
		{
			jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(classes));
			wrapper::dbg_log(xorstr_("ClassLoader is null"));
			std::exit(0);
		}

		jclass cl_class = get_env()->FindClass(xorstr_("java/lang/ClassLoader"));
		findclass_md = get_env()->GetMethodID(cl_class, xorstr_("loadClass"), xorstr_("(Ljava/lang/String;)Ljava/lang/Class;"));
		classloader_obj = get_env()->NewGlobalRef(cl);

		get_env()->DeleteLocalRef(cl_class);
		get_env()->DeleteLocalRef(cl);
		jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(classes));
		return;
	}

	jclass launchwrapper_cls = get_env()->FindClass(xorstr_("net/minecraft/launchwrapper/LaunchClassLoader"));
	if (get_env()->ExceptionCheck())
	{
		get_env()->ExceptionClear();
		launchwrapper_cls = nullptr;
	}

	if (launchwrapper_cls)
	{

		jclass launch_cls = get_env()->FindClass(xorstr_("net/minecraft/launchwrapper/Launch"));
		if (!launch_cls)
		{
			get_env()->ExceptionClear();
			get_env()->DeleteLocalRef(launchwrapper_cls);
			std::exit(0);
		}

		auto classloader_fid = get_env()->GetStaticFieldID(launch_cls, xorstr_("classLoader"), xorstr_("Lnet/minecraft/launchwrapper/LaunchClassLoader;"));
		findclass_md = get_env()->GetMethodID(launchwrapper_cls, xorstr_("findClass"), xorstr_("(Ljava/lang/String;)Ljava/lang/Class;"));
		classloader_obj = get_env()->NewGlobalRef(get_env()->GetStaticObjectField(launch_cls, classloader_fid));

		get_env()->DeleteLocalRef(launchwrapper_cls);
		get_env()->DeleteLocalRef(launch_cls);
		return;
	}

	BADLION_CLIENT = true;
	wrapper::dbg_log(xorstr_("Vanilla MC detected — using Notch mappings"));

	jint v_class_count = 0;
	jclass* v_classes = nullptr;
	jvmti_env->GetLoadedClasses(&v_class_count, &v_classes);

	jclass mc_cls = nullptr;
	for (jint i = 0; i < v_class_count; i++)
	{
		char* sig = nullptr;
		jvmti_env->GetClassSignature(v_classes[i], &sig, nullptr);
		if (sig)
		{
			if (std::strcmp(sig, xorstr_("Lave;")) == 0)
			{
				mc_cls = v_classes[i];
				sdk::version = sdk::e_version::v1_8_9;
				jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
				break;
			}
			jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(sig));
		}
	}

	if (!mc_cls)
	{
		jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(v_classes));
		wrapper::dbg_log(xorstr_("Vanilla: Minecraft class (Lave;) not found — only 1.8.9 supported"));
		std::exit(0);
	}

	jclass cls_class = get_env()->FindClass(xorstr_("java/lang/Class"));
	jmethodID get_cl_mid = get_env()->GetMethodID(cls_class, xorstr_("getClassLoader"), xorstr_("()Ljava/lang/ClassLoader;"));
	jobject cl = get_env()->CallObjectMethod(mc_cls, get_cl_mid);
	get_env()->DeleteLocalRef(cls_class);

	if (!cl)
	{
		jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(v_classes));
		wrapper::dbg_log(xorstr_("Vanilla: ClassLoader is null"));
		std::exit(0);
	}

	jclass cl_class = get_env()->FindClass(xorstr_("java/lang/ClassLoader"));
	findclass_md = get_env()->GetMethodID(cl_class, xorstr_("loadClass"), xorstr_("(Ljava/lang/String;)Ljava/lang/Class;"));
	classloader_obj = get_env()->NewGlobalRef(cl);

	get_env()->DeleteLocalRef(cl_class);
	get_env()->DeleteLocalRef(cl);
	jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(v_classes));

	dump_vanilla_classes();
}

void welllovely::c_welllovely::hook()
{
	if (SKIP_RENDER_HOOKS)
		return;

	auto swap_buffers_ptr = wrapper::get_proc_address(xorstr_("SwapBuffers"), wrapper::get_module_handle(xorstr_("Gdi32.dll")));

	if (CUSTOM_CLIENT)
	{
		veh::install(swap_buffers_ptr, hooks::swap_buffers_hk);
	}
	else
	{
		MH_Initialize();
		MH_CreateHook(swap_buffers_ptr, hooks::swap_buffers_hk, reinterpret_cast<void**>(&hooks::oswap_buffers));
		MH_EnableHook(MH_ALL_HOOKS);
	}
}

std::unique_ptr<welllovely::c_welllovely> welllovely::instance;
