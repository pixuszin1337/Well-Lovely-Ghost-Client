#include "../headers/invwalk.h"

static void set_keybind_pressed(JNIEnv* env, jobject mc, const char* mcp_name, const char* srg_name, const char* notch_name, bool pressed)
{
	auto mc_cls = env->GetObjectClass(mc);
	auto gs_fid = env->GetFieldID(mc_cls,
		CUSTOM_CLIENT ? xorstr_("gameSettings") : BADLION_CLIENT ? xorstr_("t") : xorstr_("field_71474_y"),
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/client/settings/GameSettings;") : xorstr_("Lavh;"));
	env->DeleteLocalRef(mc_cls);
	if (!gs_fid) { env->ExceptionClear(); return; }

	auto gs = env->GetObjectField(mc, gs_fid);
	if (!gs) return;

	auto gs_cls = env->GetObjectClass(gs);
	auto kb_fid = env->GetFieldID(gs_cls,
		CUSTOM_CLIENT ? mcp_name : BADLION_CLIENT ? notch_name : srg_name,
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/client/settings/KeyBinding;") : xorstr_("Lavb;"));
	env->DeleteLocalRef(gs_cls);

	if (!kb_fid) { env->ExceptionClear(); env->DeleteLocalRef(gs); return; }

	auto kb = env->GetObjectField(gs, kb_fid);
	env->DeleteLocalRef(gs);
	if (!kb) return;

	auto kb_cls = env->GetObjectClass(kb);
	auto pressed_fid = env->GetFieldID(kb_cls,
		CUSTOM_CLIENT ? xorstr_("pressed") : BADLION_CLIENT ? xorstr_("h") : xorstr_("field_74513_e"),
		xorstr_("Z"));
	env->DeleteLocalRef(kb_cls);

	if (!pressed_fid) { env->ExceptionClear(); env->DeleteLocalRef(kb); return; }

	env->SetBooleanField(kb, pressed_fid, static_cast<jboolean>(pressed));
	env->DeleteLocalRef(kb);
}

void c_invwalk::on_tick(std::shared_ptr<c_context> ctx)
{
	if (!ctx->minecraft || !ctx->local->get_object())
		return;

	if (ctx->ingame)
		return;

	auto env = welllovely::instance->get_env();

	bool w = GetAsyncKeyState('W') & 0x8000;
	bool s = GetAsyncKeyState('S') & 0x8000;
	bool a = GetAsyncKeyState('A') & 0x8000;
	bool d = GetAsyncKeyState('D') & 0x8000;

	set_keybind_pressed(env, ctx->minecraft, "keyBindForward",  "field_71351_l", "i",  w);
	set_keybind_pressed(env, ctx->minecraft, "keyBindBack",     "field_71368_g", "n",  s);
	set_keybind_pressed(env, ctx->minecraft, "keyBindLeft",     "field_71370_h", "j",  a);
	set_keybind_pressed(env, ctx->minecraft, "keyBindRight",    "field_71366_i", "k",  d);
	set_keybind_pressed(env, ctx->minecraft, "keyBindJump",     "field_71352_k", "o",  GetAsyncKeyState(VK_SPACE) & 0x8000);
	set_keybind_pressed(env, ctx->minecraft, "keyBindSprint",   "field_151444_V","au", GetAsyncKeyState(VK_CONTROL) & 0x8000);
}
