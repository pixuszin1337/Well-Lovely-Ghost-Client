#include "minecraft.h"
#include "../../main/welllovely.h"
#include "../utils/wl_log.h"

std::unique_ptr<sdk::c_minecraft> sdk::instance;

void sdk::detect_version()
{
	if (sdk::version != e_version::unknown)
		return;

	auto cls = welllovely::instance->find_class_quiet(xorstr_("net.minecraft.block.state.IBlockState"));
	if (cls)
	{
		sdk::version = e_version::v1_8_9;
		welllovely::instance->get_env()->DeleteLocalRef(cls);
	}
	else
	{
		sdk::version = e_version::v1_7_10;
	}
}

jobject sdk::c_minecraft::get_minecraft()
{
	jobject res;
	if (!BADLION_CLIENT) {
		auto cls = welllovely::instance->find_class(xorstr_("net.minecraft.client.Minecraft"));
		auto mc_fid = welllovely::instance->get_env()->GetStaticFieldID(cls, CUSTOM_CLIENT ? xorstr_("theMinecraft") : xorstr_("field_71432_P"), xorstr_("Lnet/minecraft/client/Minecraft;"));

		res = welllovely::instance->get_env()->GetStaticObjectField(cls, mc_fid);

		welllovely::instance->get_env()->DeleteLocalRef(cls);

		return res;
	}

	auto cls = welllovely::instance->find_class(xorstr_("ave"));
	auto ave = welllovely::instance->get_env()->GetStaticFieldID(cls, xorstr_("S"), xorstr_("Lave;"));

	res = welllovely::instance->get_env()->GetStaticObjectField(cls, ave);

	welllovely::instance->get_env()->DeleteLocalRef(cls);

	return res;
}

jobject sdk::c_minecraft::get_player(jobject mc) {
	if (!mc) return nullptr;
	jobject res;

	if (!BADLION_CLIENT) {
		auto env = welllovely::instance->get_env();
		auto cls = welllovely::instance->find_class(xorstr_("net.minecraft.client.Minecraft"));

		auto player_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("thePlayer") : xorstr_("field_71439_g"),
			sdk::pick(xorstr_("Lnet/minecraft/client/entity/EntityClientPlayerMP;"),
			          xorstr_("Lnet/minecraft/client/entity/EntityPlayerSP;")));

		env->DeleteLocalRef(cls);

		if (!player_fid)
		{
			env->ExceptionClear();
			return nullptr;
		}

		return env->GetObjectField(mc, player_fid);
	}

	auto cls = welllovely::instance->find_class(xorstr_("ave"));
	auto player_fid = welllovely::instance->get_env()->GetFieldID(cls, xorstr_("h"), xorstr_("Lbew;"));

	res = welllovely::instance->get_env()->GetObjectField(mc, player_fid);

	welllovely::instance->get_env()->DeleteLocalRef(cls);

	return res;
}

jobject sdk::c_minecraft::get_world(jobject mc) {
	if (!mc) return nullptr;
	jobject res;

	if (!BADLION_CLIENT) {
		auto cls = welllovely::instance->find_class(xorstr_("net.minecraft.client.Minecraft"));
		auto world_fid = welllovely::instance->get_env()->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("theWorld") : xorstr_("field_71441_e"), xorstr_("Lnet/minecraft/client/multiplayer/WorldClient;"));

		res = welllovely::instance->get_env()->GetObjectField(mc, world_fid);

		welllovely::instance->get_env()->DeleteLocalRef(cls);
		return res;
	}

	auto cls = welllovely::instance->find_class(xorstr_("ave"));
	auto world_fid = welllovely::instance->get_env()->GetFieldID(cls, xorstr_("f"), xorstr_("Lbdb;"));

	res = welllovely::instance->get_env()->GetObjectField(mc, world_fid);

	welllovely::instance->get_env()->DeleteLocalRef(cls);

	return res;
}

jobject sdk::c_minecraft::get_current_screen(jobject mc) {
	if (!mc) return nullptr;
	jobject res;

	if (!BADLION_CLIENT) {
		auto cls = welllovely::instance->find_class(xorstr_("net.minecraft.client.Minecraft"));
		auto current_screen_fid = welllovely::instance->get_env()->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("currentScreen") : xorstr_("field_71462_r"), xorstr_("Lnet/minecraft/client/gui/GuiScreen;"));

		res = welllovely::instance->get_env()->GetObjectField(mc, current_screen_fid);

		welllovely::instance->get_env()->DeleteLocalRef(cls);
		return res;
	}

	auto cls = welllovely::instance->find_class(xorstr_("ave"));
	auto current_screen_fid = welllovely::instance->get_env()->GetFieldID(cls, xorstr_("m"), xorstr_("Laxu;"));

	res = welllovely::instance->get_env()->GetObjectField(mc, current_screen_fid);

	welllovely::instance->get_env()->DeleteLocalRef(cls);

	return res;
}

jobject sdk::c_minecraft::get_entity_over(jobject mc) {
	if (!mc) return nullptr;
	jobject res = nullptr;

	auto cls = welllovely::instance->find_class(!BADLION_CLIENT ? xorstr_("net.minecraft.client.Minecraft") : xorstr_("ave"));

	auto object_mouse_over_fid = welllovely::instance->get_env()->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("objectMouseOver") : !BADLION_CLIENT ? xorstr_("field_71476_x") : xorstr_("s"), !BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/MovingObjectPosition;") : xorstr_("Lauh;"));

	auto object_mouse_over = welllovely::instance->get_env()->GetObjectField(mc, object_mouse_over_fid);

	if (!object_mouse_over)
		return res;

	auto mop_entityhit_cls = welllovely::instance->get_env()->GetObjectClass(object_mouse_over);

	auto entity_hit_fid = welllovely::instance->get_env()->GetFieldID(mop_entityhit_cls, CUSTOM_CLIENT ? xorstr_("entityHit") : !BADLION_CLIENT ? xorstr_("field_72308_g") : xorstr_("d"), !BADLION_CLIENT ? xorstr_("Lnet/minecraft/entity/Entity;") : xorstr_("Lpk;"));

	welllovely::instance->get_env()->DeleteLocalRef(cls);
	welllovely::instance->get_env()->DeleteLocalRef(mop_entityhit_cls);

	res = welllovely::instance->get_env()->GetObjectField(object_mouse_over, entity_hit_fid);

	welllovely::instance->get_env()->DeleteLocalRef(object_mouse_over);

	return res;
}

void sdk::c_minecraft::click_mouse(jobject mc)
{
	if (!mc)
		return;

	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(mc);

	auto lcc_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("leftClickCounter") : BADLION_CLIENT ? xorstr_("ag") : xorstr_("field_71429_W"), xorstr_("I"));

	if (lcc_fid)
		env->SetIntField(mc, lcc_fid, 0);
	else
		env->ExceptionClear();

	if (CUSTOM_CLIENT)
	{
		env->DeleteLocalRef(cls);
		HWND hwnd = welllovely::hooks::mc_hwnd;
		if (hwnd)
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			LPARAM lp = MAKELPARAM(rc.right / 2, rc.bottom / 2);
			PostMessageA(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lp);
			PostMessageA(hwnd, WM_LBUTTONUP, 0, lp);
		}
	}
	else
	{
		auto click_mid = env->GetMethodID(cls,
			BADLION_CLIENT ? xorstr_("aw") : xorstr_("func_147116_af"), xorstr_("()V"));

		env->DeleteLocalRef(cls);

		if (!click_mid)
		{
			env->ExceptionClear();
			return;
		}

		env->CallVoidMethod(mc, click_mid);
	}
}

float sdk::c_minecraft::get_partial_ticks(jobject mc)
{
	if (!mc)
		return 1.0f;

	auto env = welllovely::instance->get_env();

	auto mc_cls = env->GetObjectClass(mc);
	auto timer_fid = env->GetFieldID(mc_cls, CUSTOM_CLIENT ? xorstr_("timer") : BADLION_CLIENT ? xorstr_("Y") : xorstr_("field_71428_T"),
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/Timer;") : xorstr_("Lavl;"));
	env->DeleteLocalRef(mc_cls);

	if (!timer_fid)
	{
		env->ExceptionClear();
		return 1.0f;
	}

	auto timer = env->GetObjectField(mc, timer_fid);
	if (!timer)
		return 1.0f;

	auto timer_cls = env->GetObjectClass(timer);
	auto partial_fid = env->GetFieldID(timer_cls, CUSTOM_CLIENT ? xorstr_("renderPartialTicks") : BADLION_CLIENT ? xorstr_("c") : xorstr_("field_74281_c"), xorstr_("F"));
	env->DeleteLocalRef(timer_cls);

	float partial = 1.0f;
	if (partial_fid)
		partial = env->GetFloatField(timer, partial_fid);
	else
		env->ExceptionClear();

	env->DeleteLocalRef(timer);
	return partial;
}

static jobject get_game_settings(JNIEnv* env, jobject mc)
{
	auto mc_cls = env->GetObjectClass(mc);
	auto gs_fid = env->GetFieldID(mc_cls,
		CUSTOM_CLIENT ? xorstr_("gameSettings") : BADLION_CLIENT ? xorstr_("t") : xorstr_("field_71474_y"),
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/client/settings/GameSettings;") : xorstr_("Lavh;"));
	env->DeleteLocalRef(mc_cls);
	if (!gs_fid)
	{
		env->ExceptionClear();
		return nullptr;
	}
	return env->GetObjectField(mc, gs_fid);
}

float sdk::c_minecraft::get_gamma(jobject mc)
{
	if (!mc)
		return 1.0f;

	auto env = welllovely::instance->get_env();
	auto gs = get_game_settings(env, mc);
	if (!gs)
		return 1.0f;

	auto gs_cls = env->GetObjectClass(gs);
	auto gamma_fid = env->GetFieldID(gs_cls, CUSTOM_CLIENT ? xorstr_("gammaSetting") : BADLION_CLIENT ? xorstr_("aJ") : xorstr_("field_74333_Y"), xorstr_("F"));
	env->DeleteLocalRef(gs_cls);

	float g = 1.0f;
	if (gamma_fid)
		g = env->GetFloatField(gs, gamma_fid);
	else
		env->ExceptionClear();

	env->DeleteLocalRef(gs);
	return g;
}

void sdk::c_minecraft::set_gamma(jobject mc, float value)
{
	if (!mc)
		return;

	auto env = welllovely::instance->get_env();
	auto gs = get_game_settings(env, mc);
	if (!gs)
		return;

	auto gs_cls = env->GetObjectClass(gs);
	auto gamma_fid = env->GetFieldID(gs_cls, CUSTOM_CLIENT ? xorstr_("gammaSetting") : BADLION_CLIENT ? xorstr_("aJ") : xorstr_("field_74333_Y"), xorstr_("F"));
	env->DeleteLocalRef(gs_cls);

	if (gamma_fid)
		env->SetFloatField(gs, gamma_fid, value);
	else
		env->ExceptionClear();

	env->DeleteLocalRef(gs);
}

bool sdk::c_minecraft::is_looking_at_block(jobject mc)
{
	if (!mc)
		return false;

	auto env = welllovely::instance->get_env();

	auto mc_cls = env->GetObjectClass(mc);
	auto mop_fid = env->GetFieldID(mc_cls,
		CUSTOM_CLIENT ? xorstr_("objectMouseOver") : BADLION_CLIENT ? xorstr_("s") : xorstr_("field_71476_x"),
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/MovingObjectPosition;") : xorstr_("Lauh;"));
	env->DeleteLocalRef(mc_cls);
	if (!mop_fid)
	{
		env->ExceptionClear();
		return false;
	}

	auto mop = env->GetObjectField(mc, mop_fid);
	if (!mop)
		return false;

	auto mop_cls = env->GetObjectClass(mop);
	auto type_fid = env->GetFieldID(mop_cls,
		CUSTOM_CLIENT ? xorstr_("typeOfHit") : BADLION_CLIENT ? xorstr_("a") : xorstr_("field_72313_a"),
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;") : xorstr_("Lauh$a;"));
	env->DeleteLocalRef(mop_cls);
	if (!type_fid)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(mop);
		return false;
	}

	auto type_obj = env->GetObjectField(mop, type_fid);
	env->DeleteLocalRef(mop);
	if (!type_obj)
		return false;

	auto enum_cls = env->GetObjectClass(type_obj);
	auto ordinal_mid = env->GetMethodID(enum_cls, xorstr_("ordinal"), xorstr_("()I"));
	env->DeleteLocalRef(enum_cls);

	bool is_block = false;
	if (ordinal_mid)
		is_block = (env->CallIntMethod(type_obj, ordinal_mid) == 1);
	else
		env->ExceptionClear();

	env->DeleteLocalRef(type_obj);
	return is_block;
}

extern jvmtiEnv* jvmti_env;

static jfieldID  s_sneak_fid   = nullptr;
static jfieldID  s_pressed_fid = nullptr;
static bool      s_sneak_resolved = false;

void sdk::c_minecraft::set_sneak_key_pressed(jobject mc, bool value)
{
	if (!mc) return;
	auto env = welllovely::instance->get_env();

	if (!s_sneak_resolved)
	{
		s_sneak_resolved = true;

		jclass kb_cls = nullptr;
		if (BADLION_CLIENT)
			kb_cls = welllovely::instance->find_class_quiet(xorstr_("avf"));
		else
			kb_cls = welllovely::instance->find_class_quiet(xorstr_("net.minecraft.client.settings.KeyBinding"));

		if (!kb_cls) return;

		char* kb_sig = nullptr;
		jvmti_env->GetClassSignature(kb_cls, &kb_sig, nullptr);
		if (!kb_sig) { env->DeleteLocalRef(kb_cls); return; }

		auto gs = get_game_settings(env, mc);
		if (!gs) { jvmti_env->Deallocate((unsigned char*)kb_sig); env->DeleteLocalRef(kb_cls); return; }

		auto gs_cls = env->GetObjectClass(gs);

		s_sneak_fid = env->GetFieldID(gs_cls,
			CUSTOM_CLIENT ? xorstr_("keyBindSneak") : BADLION_CLIENT ? xorstr_("ar") : xorstr_("field_74311_sneak"),
			kb_sig);

		if (!s_sneak_fid && env->ExceptionCheck()) env->ExceptionClear();

		if (s_sneak_fid)
		{
			s_pressed_fid = env->GetFieldID(kb_cls,
				CUSTOM_CLIENT ? xorstr_("pressed") : BADLION_CLIENT ? xorstr_("h") : xorstr_("field_74513_e"),
				xorstr_("Z"));

			if (!s_pressed_fid && env->ExceptionCheck()) env->ExceptionClear();
		}

		jvmti_env->Deallocate((unsigned char*)kb_sig);
		env->DeleteLocalRef(kb_cls);
		env->DeleteLocalRef(gs_cls);
		env->DeleteLocalRef(gs);
	}

	if (!s_sneak_fid || !s_pressed_fid) return;

	auto gs = get_game_settings(env, mc);
	if (!gs) return;

	auto kb = env->GetObjectField(gs, s_sneak_fid);
	env->DeleteLocalRef(gs);
	if (!kb) return;

	env->SetBooleanField(kb, s_pressed_fid, value ? JNI_TRUE : JNI_FALSE);
	env->DeleteLocalRef(kb);
}

static jfieldID s_move_fids[4] = { nullptr, nullptr, nullptr, nullptr };
static jfieldID s_move_pressed_fid = nullptr;
static bool     s_move_resolved = false;

bool sdk::c_minecraft::get_move_input(jobject mc, float& forward, float& strafe)
{
	forward = 0.0f;
	strafe = 0.0f;
	if (!mc) return false;
	auto env = welllovely::instance->get_env();

	if (!s_move_resolved)
	{
		s_move_resolved = true;

		jclass kb_cls = nullptr;
		if (BADLION_CLIENT)
			kb_cls = welllovely::instance->find_class_quiet(xorstr_("avf"));
		else
			kb_cls = welllovely::instance->find_class_quiet(xorstr_("net.minecraft.client.settings.KeyBinding"));

		if (!kb_cls) return false;

		char* kb_sig = nullptr;
		jvmti_env->GetClassSignature(kb_cls, &kb_sig, nullptr);
		if (!kb_sig) { env->DeleteLocalRef(kb_cls); return false; }

		auto gs = get_game_settings(env, mc);
		if (!gs) { jvmti_env->Deallocate((unsigned char*)kb_sig); env->DeleteLocalRef(kb_cls); return false; }

		auto gs_cls = env->GetObjectClass(gs);

		if (!BADLION_CLIENT)
		{
			s_move_fids[0] = env->GetFieldID(gs_cls, CUSTOM_CLIENT ? xorstr_("keyBindForward") : xorstr_("field_74351_w"), kb_sig);
			if (!s_move_fids[0] && env->ExceptionCheck()) env->ExceptionClear();

			s_move_fids[1] = env->GetFieldID(gs_cls, CUSTOM_CLIENT ? xorstr_("keyBindBack") : xorstr_("field_74368_aC"), kb_sig);
			if (!s_move_fids[1] && env->ExceptionCheck()) env->ExceptionClear();

			s_move_fids[2] = env->GetFieldID(gs_cls, CUSTOM_CLIENT ? xorstr_("keyBindLeft") : xorstr_("field_74370_x"), kb_sig);
			if (!s_move_fids[2] && env->ExceptionCheck()) env->ExceptionClear();

			s_move_fids[3] = env->GetFieldID(gs_cls, CUSTOM_CLIENT ? xorstr_("keyBindRight") : xorstr_("field_74366_y"), kb_sig);
			if (!s_move_fids[3] && env->ExceptionCheck()) env->ExceptionClear();
		}

		if (s_move_fids[0] || s_move_fids[1] || s_move_fids[2] || s_move_fids[3])
		{
			s_move_pressed_fid = env->GetFieldID(kb_cls,
				CUSTOM_CLIENT ? xorstr_("pressed") : BADLION_CLIENT ? xorstr_("h") : xorstr_("field_74513_e"),
				xorstr_("Z"));

			if (!s_move_pressed_fid && env->ExceptionCheck())
				env->ExceptionClear();
		}

		jvmti_env->Deallocate((unsigned char*)kb_sig);
		env->DeleteLocalRef(kb_cls);
		env->DeleteLocalRef(gs_cls);
		env->DeleteLocalRef(gs);
	}

	if (!s_move_pressed_fid) return false;

	auto gs = get_game_settings(env, mc);
	if (!gs) return false;

	bool ok = true;
	bool pressed[4] = { false, false, false, false };
	for (int i = 0; i < 4; i++)
	{
		if (!s_move_fids[i])
		{
			ok = false;
			continue;
		}

		auto kb = env->GetObjectField(gs, s_move_fids[i]);
		if (!kb) { ok = false; if (env->ExceptionCheck()) env->ExceptionClear(); continue; }

		pressed[i] = env->GetBooleanField(kb, s_move_pressed_fid) != JNI_FALSE;
		env->DeleteLocalRef(kb);
	}
	env->DeleteLocalRef(gs);

	if (!ok) return false;

	forward = (pressed[0] ? 1.0f : 0.0f) - (pressed[1] ? 1.0f : 0.0f);
	strafe  = (pressed[2] ? 1.0f : 0.0f) - (pressed[3] ? 1.0f : 0.0f);
	return true;
}

static jfieldID s_use_fid       = nullptr;
static jfieldID s_use_pressed_fid = nullptr;
static bool     s_use_resolved  = false;

void sdk::c_minecraft::set_use_key_pressed(jobject mc, bool value)
{
	if (!mc) return;
	auto env = welllovely::instance->get_env();

	if (!s_use_resolved)
	{
		s_use_resolved = true;

		jclass kb_cls = nullptr;
		if (BADLION_CLIENT)
			kb_cls = welllovely::instance->find_class_quiet(xorstr_("avf"));
		else
			kb_cls = welllovely::instance->find_class_quiet(xorstr_("net.minecraft.client.settings.KeyBinding"));

		if (!kb_cls) return;

		char* kb_sig = nullptr;
		jvmti_env->GetClassSignature(kb_cls, &kb_sig, nullptr);
		if (!kb_sig) { env->DeleteLocalRef(kb_cls); return; }

		auto gs = get_game_settings(env, mc);
		if (!gs) { jvmti_env->Deallocate((unsigned char*)kb_sig); env->DeleteLocalRef(kb_cls); return; }

		auto gs_cls = env->GetObjectClass(gs);

		s_use_fid = env->GetFieldID(gs_cls,
			CUSTOM_CLIENT ? xorstr_("keyBindUse") : BADLION_CLIENT ? xorstr_("r") : xorstr_("field_74324_g"),
			kb_sig);

		if (!s_use_fid && env->ExceptionCheck()) env->ExceptionClear();

		if (s_use_fid)
		{
			s_use_pressed_fid = env->GetFieldID(kb_cls,
				CUSTOM_CLIENT ? xorstr_("pressed") : BADLION_CLIENT ? xorstr_("h") : xorstr_("field_74513_e"),
				xorstr_("Z"));

			if (!s_use_pressed_fid && env->ExceptionCheck()) env->ExceptionClear();
		}

		jvmti_env->Deallocate((unsigned char*)kb_sig);
		env->DeleteLocalRef(kb_cls);
		env->DeleteLocalRef(gs_cls);
		env->DeleteLocalRef(gs);
	}

	if (!s_use_fid || !s_use_pressed_fid) return;

	auto gs = get_game_settings(env, mc);
	if (!gs) return;

	auto kb = env->GetObjectField(gs, s_use_fid);
	env->DeleteLocalRef(gs);
	if (!kb) return;

	env->SetBooleanField(kb, s_use_pressed_fid, value ? JNI_TRUE : JNI_FALSE);
	env->DeleteLocalRef(kb);
}

void sdk::c_minecraft::right_click_mouse(jobject mc)
{
	if (!mc)
		return;

	if (CUSTOM_CLIENT)
	{
		HWND hwnd = welllovely::hooks::mc_hwnd;
		if (hwnd)
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			LPARAM lp = MAKELPARAM(rc.right / 2, rc.bottom / 2);
			PostMessageA(hwnd, WM_RBUTTONDOWN, MK_RBUTTON, lp);
		}
		return;
	}

	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(mc);

	auto mid = env->GetMethodID(cls,
		BADLION_CLIENT ? xorstr_("ak") : xorstr_("func_147121_ag"), xorstr_("()V"));

	env->DeleteLocalRef(cls);

	if (!mid)
	{
		env->ExceptionClear();
		return;
	}

	env->CallVoidMethod(mc, mid);
	if (env->ExceptionCheck())
		env->ExceptionClear();
}

void sdk::c_minecraft::right_click_release(jobject mc)
{
	if (!mc)
		return;

	if (CUSTOM_CLIENT)
	{
		HWND hwnd = welllovely::hooks::mc_hwnd;
		if (hwnd)
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			LPARAM lp = MAKELPARAM(rc.right / 2, rc.bottom / 2);
			PostMessageA(hwnd, WM_RBUTTONUP, 0, lp);
		}
	}

}

void sdk::c_minecraft::window_click(jobject mc, int slot, int mouse_btn, int mode)
{
	if (!mc)
		return;

	auto env = welllovely::instance->get_env();
	if (!env)
		return;

	send_open_inventory_packet(mc);

	auto player = get_player(mc);
	if (!player)
		return;

	auto mc_cls = env->GetObjectClass(mc);
	auto pc_fid = env->GetFieldID(mc_cls,
		CUSTOM_CLIENT ? xorstr_("playerController") : BADLION_CLIENT ? xorstr_("c") : xorstr_("field_71442_b"),
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/client/multiplayer/PlayerControllerMP;") : xorstr_("Lbje;"));
	env->DeleteLocalRef(mc_cls);

	if (!pc_fid)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(player);
		wl::log("WC: playerController fid FALHOU");
		return;
	}

	auto pc = env->GetObjectField(mc, pc_fid);
	if (!pc)
	{
		env->DeleteLocalRef(player);
		wl::log("WC: playerController obj nulo");
		return;
	}

	auto pc_cls = env->GetObjectClass(pc);
	auto mid = env->GetMethodID(pc_cls,
		CUSTOM_CLIENT ? xorstr_("windowClick") : BADLION_CLIENT ? xorstr_("a") : xorstr_("func_78764_a"),
		xorstr_("(IIIILnet/minecraft/entity/player/EntityPlayer;)Lnet/minecraft/item/ItemStack;"));
	env->DeleteLocalRef(pc_cls);

	if (!mid)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(pc);
		env->DeleteLocalRef(player);
		wl::log("WC: windowClick mid FALHOU");
		return;
	}

	wl::log("WC: click slot=%d btn=%d mode=%d", slot, mouse_btn, mode);
	auto res = env->CallObjectMethod(pc, mid, (jint)0, (jint)slot, (jint)mouse_btn, (jint)mode, player);
	if (env->ExceptionCheck())
		env->ExceptionClear();

	if (res)
		env->DeleteLocalRef(res);

	env->DeleteLocalRef(pc);
	env->DeleteLocalRef(player);
}

void sdk::c_minecraft::send_open_inventory_packet(jobject mc)
{
	if (!mc)
		return;

	auto env = welllovely::instance->get_env();
	if (!env)
		return;

	auto player = get_player(mc);
	if (!player)
		return;

	auto enum_cls = welllovely::instance->find_class_quiet(
		!BADLION_CLIENT ? xorstr_("net.minecraft.network.play.client.C16PacketClientStatus$EnumState") : xorstr_("gi$a"));
	if (!enum_cls)
	{
		env->DeleteLocalRef(player);
		wl::log("C16: classe do enum NAO achada");
		return;
	}

	auto state_fid = env->GetStaticFieldID(enum_cls, xorstr_("OPEN_INVENTORY_ACHIEVEMENT"),
		xorstr_("Lnet/minecraft/network/play/client/C16PacketClientStatus$EnumState;"));
	if (!state_fid)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(enum_cls);
		env->DeleteLocalRef(player);
		wl::log("C16: campo do enum NAO achado");
		return;
	}

	auto state = env->GetStaticObjectField(enum_cls, state_fid);
	env->DeleteLocalRef(enum_cls);

	if (!state)
	{
		env->DeleteLocalRef(player);
		return;
	}

	auto pkt_cls = welllovely::instance->find_class_quiet(
		!BADLION_CLIENT ? xorstr_("net.minecraft.network.play.client.C16PacketClientStatus") : xorstr_("gi"));
	if (!pkt_cls)
	{
		env->DeleteLocalRef(state);
		env->DeleteLocalRef(player);
		return;
	}

	auto ctor = env->GetMethodID(pkt_cls, xorstr_("<init>"),
		xorstr_("(Lnet/minecraft/network/play/client/C16PacketClientStatus$EnumState;)V"));
	if (!ctor)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(pkt_cls);
		env->DeleteLocalRef(state);
		env->DeleteLocalRef(player);
		wl::log("C16: ctor NAO achado");
		return;
	}

	auto pkt = env->NewObject(pkt_cls, ctor, state);
	env->DeleteLocalRef(pkt_cls);

	if (!pkt)
	{
		if (env->ExceptionCheck()) env->ExceptionClear();
		env->DeleteLocalRef(state);
		env->DeleteLocalRef(player);
		wl::log("C16: NewObject falhou");
		return;
	}

	auto p_cls = env->GetObjectClass(player);
	auto sq_fid = env->GetFieldID(p_cls,
		CUSTOM_CLIENT ? xorstr_("sendQueue") : BADLION_CLIENT ? xorstr_("g") : xorstr_("field_71174_a"),
		xorstr_("Lnet/minecraft/client/network/NetHandlerPlayClient;"));
	env->DeleteLocalRef(p_cls);

	if (!sq_fid)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(pkt);
		env->DeleteLocalRef(state);
		env->DeleteLocalRef(player);
		wl::log("C16: sendQueue fid NAO achado");
		return;
	}

	auto sendqueue = env->GetObjectField(player, sq_fid);
	if (sendqueue)
	{
		auto sq_cls = env->GetObjectClass(sendqueue);
		auto send_mid = env->GetMethodID(sq_cls,
			CUSTOM_CLIENT ? xorstr_("addToSendQueue") : BADLION_CLIENT ? xorstr_("a") : xorstr_("func_147297_a"),
			xorstr_("(Lnet/minecraft/network/Packet;)V"));
		env->DeleteLocalRef(sq_cls);

		if (send_mid)
		{
			env->CallVoidMethod(sendqueue, send_mid, pkt);
			wl::log("C16: enviado OK");
		}
		else
			env->ExceptionClear();

		env->DeleteLocalRef(sendqueue);
	}

	env->DeleteLocalRef(pkt);
	env->DeleteLocalRef(state);
	env->DeleteLocalRef(player);
}
