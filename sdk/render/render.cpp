#include "render.h"

#include "../../main/welllovely.h"
#include "../minecraft/minecraft.h"

#include <cstring>

#ifdef _DEBUG
#include <cstdio>

void sdk::debug_probe_method(const char* class_name, const char* method_name, const char* sig)
{
	auto env = welllovely::instance->get_env();
	jclass cls = welllovely::instance->find_class_quiet(class_name);
	if (!cls) { printf("[PROBE] classe NAO achada: %s\n", class_name); return; }
	jmethodID mid = env->GetMethodID(cls, method_name, sig);
	bool found = (mid != nullptr);
	if (!found) env->ExceptionClear();
	env->DeleteLocalRef(cls);
	printf("[PROBE] %s . %s %s  ->  %s\n", class_name, method_name, sig, found ? "ACHADO" : "NAO ACHADO");
}

void sdk::debug_dump_fields(const char* class_name)
{
	auto env = welllovely::instance->get_env();
	jclass cls = welllovely::instance->find_class_quiet(class_name);
	if (!cls) { printf("[DUMP] classe NAO encontrada: %s\n", class_name); return; }
	jclass class_cls = env->GetObjectClass(cls);
	jmethodID m_getFields = env->GetMethodID(class_cls, "getDeclaredFields", "()[Ljava/lang/reflect/Field;");
	jmethodID m_classGetName = env->GetMethodID(class_cls, "getName", "()Ljava/lang/String;");
	jclass field_cls = env->FindClass("java/lang/reflect/Field");
	jmethodID m_fieldGetName = env->GetMethodID(field_cls, "getName", "()Ljava/lang/String;");
	jmethodID m_fieldGetType = env->GetMethodID(field_cls, "getType", "()Ljava/lang/Class;");
	jmethodID m_fieldGetMods = env->GetMethodID(field_cls, "getModifiers", "()I");
	jobjectArray fields = static_cast<jobjectArray>(env->CallObjectMethod(cls, m_getFields));
	if (!fields) { printf("[DUMP] getDeclaredFields retornou null pra %s\n", class_name); return; }
	int n = env->GetArrayLength(fields);
	printf("[DUMP] === %s : %d campos ===\n", class_name, n);
	for (int i = 0; i < n; ++i)
	{
		jobject f = env->GetObjectArrayElement(fields, i);
		jstring jname = static_cast<jstring>(env->CallObjectMethod(f, m_fieldGetName));
		jobject type = env->CallObjectMethod(f, m_fieldGetType);
		jstring jtype = static_cast<jstring>(env->CallObjectMethod(type, m_classGetName));
		jint mods = env->CallIntMethod(f, m_fieldGetMods);
		const char* nm = env->GetStringUTFChars(jname, nullptr);
		const char* tn = env->GetStringUTFChars(jtype, nullptr);
		printf("[DUMP]   %s%s : %s\n", (mods & 0x8) ? "static " : "", nm, tn);
		env->ReleaseStringUTFChars(jname, nm);
		env->ReleaseStringUTFChars(jtype, tn);
		env->DeleteLocalRef(f);
		env->DeleteLocalRef(jname);
		env->DeleteLocalRef(type);
		env->DeleteLocalRef(jtype);
	}
	env->DeleteLocalRef(fields);
	env->DeleteLocalRef(field_cls);
	env->DeleteLocalRef(class_cls);
	env->DeleteLocalRef(cls);
}
#endif

sdk::s_box2d sdk::project_box(const s_view& view, const sdk::vec3d& feet, double width, double height)
{
	s_box2d b{};
	b.box_valid = false;
	b.feet_valid = false;
	b.head_valid = false;

	const double hw = width * 0.5;

	const double corners[8][3] = {
		{ feet.x - hw, feet.y,          feet.z - hw }, { feet.x - hw, feet.y,          feet.z + hw },
		{ feet.x + hw, feet.y,          feet.z - hw }, { feet.x + hw, feet.y,          feet.z + hw },
		{ feet.x - hw, feet.y + height, feet.z - hw }, { feet.x - hw, feet.y + height, feet.z + hw },
		{ feet.x + hw, feet.y + height, feet.z - hw }, { feet.x + hw, feet.y + height, feet.z + hw },
	};

	float minx = 1e9f, miny = 1e9f, maxx = -1e9f, maxy = -1e9f;
	bool all = true;

	for (auto& c : corners)
	{
		sdk::vec3d rel{ c[0] - view.render_pos.x, c[1] - view.render_pos.y, c[2] - view.render_pos.z };

		float sx, sy;
		if (!sdk::util::world_to_screen(rel, view.modelview, view.projection, view.viewport, sx, sy))
		{
			all = false;
			break;
		}

		if (sx < minx) minx = sx;
		if (sy < miny) miny = sy;
		if (sx > maxx) maxx = sx;
		if (sy > maxy) maxy = sy;
	}

	if (all)
	{
		b.box_valid = true;
		b.min_x = minx; b.min_y = miny; b.max_x = maxx; b.max_y = maxy;
	}

	{
		sdk::vec3d rel{ feet.x - view.render_pos.x, feet.y - view.render_pos.y, feet.z - view.render_pos.z };
		b.feet_valid = sdk::util::world_to_screen(rel, view.modelview, view.projection, view.viewport, b.feet_x, b.feet_y);
	}

	{
		sdk::vec3d rel{ feet.x - view.render_pos.x, feet.y + height - view.render_pos.y, feet.z - view.render_pos.z };
		b.head_valid = sdk::util::world_to_screen(rel, view.modelview, view.projection, view.viewport, b.head_x, b.head_y);
	}

	return b;
}

sdk::s_view sdk::c_render::capture()
{
	s_view v{};
	v.valid = false;

	auto env = welllovely::instance->get_env();

	auto ari_cls = welllovely::instance->find_class_quiet(
		BADLION_CLIENT ? xorstr_("auz") : xorstr_("net.minecraft.client.renderer.ActiveRenderInfo"));
	if (!ari_cls)
		return v;
	v.stage = 1;

	auto mv_fid = env->GetStaticFieldID(ari_cls, CUSTOM_CLIENT ? xorstr_("MODELVIEW") : BADLION_CLIENT ? xorstr_("b") : sdk::pick(xorstr_("field_74594_j"), xorstr_("field_178812_b")), xorstr_("Ljava/nio/FloatBuffer;"));
	auto pr_fid = env->GetStaticFieldID(ari_cls, CUSTOM_CLIENT ? xorstr_("PROJECTION") : BADLION_CLIENT ? xorstr_("c") : sdk::pick(xorstr_("field_74595_k"), xorstr_("field_178813_c")), xorstr_("Ljava/nio/FloatBuffer;"));
	auto vp_fid = env->GetStaticFieldID(ari_cls, CUSTOM_CLIENT ? xorstr_("VIEWPORT") : BADLION_CLIENT ? xorstr_("a") : sdk::pick(xorstr_("field_74597_i"), xorstr_("field_178814_a")), xorstr_("Ljava/nio/IntBuffer;"));

	if (!mv_fid || !pr_fid || !vp_fid)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(ari_cls);
		return v;
	}
	v.stage = 2;

	auto mv_buf = env->GetStaticObjectField(ari_cls, mv_fid);
	auto pr_buf = env->GetStaticObjectField(ari_cls, pr_fid);
	auto vp_buf = env->GetStaticObjectField(ari_cls, vp_fid);
	env->DeleteLocalRef(ari_cls);

	if (!mv_buf || !pr_buf || !vp_buf)
		return v;
	v.stage = 3;

	auto mv = static_cast<float*>(env->GetDirectBufferAddress(mv_buf));
	auto pr = static_cast<float*>(env->GetDirectBufferAddress(pr_buf));
	auto vp = static_cast<int*>(env->GetDirectBufferAddress(vp_buf));

	if (mv && pr && vp)
	{
		memcpy(v.modelview, mv, sizeof(float) * 16);
		memcpy(v.projection, pr, sizeof(float) * 16);
		memcpy(v.viewport, vp, sizeof(int) * 4);
	}

	env->DeleteLocalRef(mv_buf);
	env->DeleteLocalRef(pr_buf);
	env->DeleteLocalRef(vp_buf);

	if (!mv || !pr || !vp)
		return v;
	v.stage = 4;

	if (sdk::version == e_version::v1_8_9 && !BADLION_CLIENT)
	{
		auto mc = sdk::instance->get_minecraft();
		if (!mc)
			return v;
		v.stage = 5;

		auto mc_cls = env->GetObjectClass(mc);
		auto rm_fid = env->GetFieldID(mc_cls, CUSTOM_CLIENT ? xorstr_("renderManager") : xorstr_("field_175616_W"), xorstr_("Lnet/minecraft/client/renderer/entity/RenderManager;"));
		env->DeleteLocalRef(mc_cls);
		if (!rm_fid)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(mc);
			return v;
		}

		auto rm_inst = env->GetObjectField(mc, rm_fid);
		env->DeleteLocalRef(mc);
		if (!rm_inst)
			return v;
		v.stage = 6;

		auto rm_cls = env->GetObjectClass(rm_inst);
		auto rx = env->GetFieldID(rm_cls, CUSTOM_CLIENT ? xorstr_("renderPosX") : xorstr_("field_78725_b"), xorstr_("D"));
		auto ry = env->GetFieldID(rm_cls, CUSTOM_CLIENT ? xorstr_("renderPosY") : xorstr_("field_78726_c"), xorstr_("D"));
		auto rz = env->GetFieldID(rm_cls, CUSTOM_CLIENT ? xorstr_("renderPosZ") : xorstr_("field_78723_d"), xorstr_("D"));
		env->DeleteLocalRef(rm_cls);

		if (rx && ry && rz)
		{
			v.render_pos.x = env->GetDoubleField(rm_inst, rx);
			v.render_pos.y = env->GetDoubleField(rm_inst, ry);
			v.render_pos.z = env->GetDoubleField(rm_inst, rz);
			v.valid = true;
			v.stage = 7;
		}
		else
			env->ExceptionClear();

		env->DeleteLocalRef(rm_inst);
		return v;
	}

	auto rm_cls = welllovely::instance->find_class_quiet(
		BADLION_CLIENT ? xorstr_("bhc") : xorstr_("net.minecraft.client.renderer.entity.RenderManager"));
	if (!rm_cls)
		return v;
	v.stage = 5;

	auto rx = env->GetStaticFieldID(rm_cls, CUSTOM_CLIENT ? xorstr_("renderPosX") : BADLION_CLIENT ? xorstr_("b") : xorstr_("field_78725_b"), xorstr_("D"));
	auto ry = env->GetStaticFieldID(rm_cls, CUSTOM_CLIENT ? xorstr_("renderPosY") : BADLION_CLIENT ? xorstr_("c") : xorstr_("field_78726_c"), xorstr_("D"));
	auto rz = env->GetStaticFieldID(rm_cls, CUSTOM_CLIENT ? xorstr_("renderPosZ") : BADLION_CLIENT ? xorstr_("d") : xorstr_("field_78723_d"), xorstr_("D"));

	if (rx && ry && rz)
	{
		v.render_pos.x = env->GetStaticDoubleField(rm_cls, rx);
		v.render_pos.y = env->GetStaticDoubleField(rm_cls, ry);
		v.render_pos.z = env->GetStaticDoubleField(rm_cls, rz);
		v.valid = true;
		v.stage = 7;
	}
	else
		env->ExceptionClear();

	env->DeleteLocalRef(rm_cls);
	return v;
}
