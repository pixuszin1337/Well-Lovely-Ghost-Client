#include "player.h"

#include "../../../main/welllovely.h"
#include "../axisaligned/axisalignedbb.h"

c_player::c_player(jobject obj)
{
	player_obj = obj;
}

c_player::~c_player()
{
	welllovely::instance->get_env()->DeleteLocalRef(player_obj);
}

bool c_player::is_invisible()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	jmethodID invisible_md = env->GetMethodID(cls, CUSTOM_CLIENT ? xorstr_("isInvisible") : BADLION_CLIENT ? xorstr_("ax") : xorstr_("func_82150_aj"), xorstr_("()Z"));

	env->DeleteLocalRef(cls);

	if (!invisible_md)
	{
		env->ExceptionClear();
		return false;
	}

	return env->CallBooleanMethod(player_obj, invisible_md);
}

bool c_player::is_sprinting()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto mid = env->GetMethodID(cls, CUSTOM_CLIENT ? xorstr_("isSprinting") : BADLION_CLIENT ? xorstr_("bE") : xorstr_("func_70051_ag"), xorstr_("()Z"));

	env->DeleteLocalRef(cls);

	if (!mid)
	{
		env->ExceptionClear();
		return false;
	}

	return env->CallBooleanMethod(player_obj, mid);
}

float c_player::get_ai_move_speed()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto mid = env->GetMethodID(cls, CUSTOM_CLIENT ? xorstr_("getAIMoveSpeed") : BADLION_CLIENT ? xorstr_("bK") : xorstr_("func_110611_ar"), xorstr_("()F"));

	env->DeleteLocalRef(cls);

	if (!mid)
	{
		env->ExceptionClear();
		return -1.0f;
	}

	return env->CallFloatMethod(player_obj, mid);
}

float c_player::get_hurt_time()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("hurtTime") : BADLION_CLIENT ? xorstr_("au") : xorstr_("field_70737_aN"), xorstr_("I"));

	env->DeleteLocalRef(cls);

	if (!fid)
	{
		env->ExceptionClear();
		return 0.0f;
	}

	return static_cast<float>(env->GetIntField(player_obj, fid));
}

float c_player::get_moveforward()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("moveForward") : BADLION_CLIENT ? xorstr_("ba") : xorstr_("field_70701_bs"), xorstr_("F"));
	env->DeleteLocalRef(cls);
	if (!fid) { env->ExceptionClear(); return 0.0f; }
	return env->GetFloatField(player_obj, fid);
}

float c_player::get_movestrafe()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("moveStrafing") : BADLION_CLIENT ? xorstr_("aZ") : xorstr_("field_70702_br"), xorstr_("F"));
	env->DeleteLocalRef(cls);
	if (!fid) { env->ExceptionClear(); return 0.0f; }
	return env->GetFloatField(player_obj, fid);
}

sdk::vec3d c_player::get_motion_vector()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto mox_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("motionX") : BADLION_CLIENT ? xorstr_("v") : xorstr_("field_70159_w"), xorstr_("D"));
	auto moy_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("motionY") : BADLION_CLIENT ? xorstr_("w") : xorstr_("field_70181_x"), xorstr_("D"));
	auto moz_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("motionZ") : BADLION_CLIENT ? xorstr_("x") : xorstr_("field_70179_y"), xorstr_("D"));
	env->DeleteLocalRef(cls);

	return sdk::vec3d{
		env->GetDoubleField(player_obj, mox_fid),
		env->GetDoubleField(player_obj, moy_fid),
		env->GetDoubleField(player_obj, moz_fid)
	};
}

void c_player::get_motion_vector(sdk::vec3d vec)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto mox_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("motionX") : BADLION_CLIENT ? xorstr_("v") : xorstr_("field_70159_w"), xorstr_("D"));
	auto moy_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("motionY") : BADLION_CLIENT ? xorstr_("w") : xorstr_("field_70181_x"), xorstr_("D"));
	auto moz_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("motionZ") : BADLION_CLIENT ? xorstr_("x") : xorstr_("field_70179_y"), xorstr_("D"));
	env->DeleteLocalRef(cls);

	env->SetDoubleField(player_obj, mox_fid, vec.x);
	env->SetDoubleField(player_obj, moy_fid, vec.y);
	env->SetDoubleField(player_obj, moz_fid, vec.z);
}

bool c_player::is_on_ground()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("onGround") : BADLION_CLIENT ? xorstr_("C") : xorstr_("field_70122_E"), xorstr_("Z"));
	env->DeleteLocalRef(cls);
	if (!fid) { env->ExceptionClear(); return false; }
	return env->GetBooleanField(player_obj, fid);
}

float c_player::get_health()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto get_health_mid = env->GetMethodID(cls, CUSTOM_CLIENT ? xorstr_("getHealth") : BADLION_CLIENT ? xorstr_("bn") : xorstr_("func_110143_aJ"), xorstr_("()F"));

	env->DeleteLocalRef(cls);

	if (!get_health_mid)
	{

		env->ExceptionClear();
		return 1337.f;
	}

	return env->CallFloatMethod(player_obj, get_health_mid);
}

sdk::vec3d c_player::get_position()
{
	auto player_class = welllovely::instance->get_env()->GetObjectClass(player_obj);

	jfieldID pos_x_fid, pos_y_fid, pos_z_fid;

	if (CUSTOM_CLIENT) {
		pos_x_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("posX"), xorstr_("D"));
		pos_y_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("posY"), xorstr_("D"));
		pos_z_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("posZ"), xorstr_("D"));
	}
	else if (!BADLION_CLIENT) {
		pos_x_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70165_t"), xorstr_("D"));
		pos_y_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70163_u"), xorstr_("D"));
		pos_z_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70161_v"), xorstr_("D"));
	}
	else {
		pos_x_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("s"), xorstr_("D"));
		pos_y_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("t"), xorstr_("D"));
		pos_z_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("u"), xorstr_("D"));
	}

	welllovely::instance->get_env()->DeleteLocalRef(player_class);

	return sdk::vec3d{
		welllovely::instance->get_env()->GetDoubleField(player_obj, pos_x_fid),
		welllovely::instance->get_env()->GetDoubleField(player_obj, pos_y_fid),
		welllovely::instance->get_env()->GetDoubleField(player_obj, pos_z_fid)
	};
}

void c_player::set_yaw(float yaw)
{
	auto player_class = welllovely::instance->get_env()->GetObjectClass(player_obj);

	jfieldID yaw_fid;
	if (CUSTOM_CLIENT) {
		yaw_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("rotationYaw"), xorstr_("F"));
	}
	else if (!BADLION_CLIENT) {
		yaw_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70177_z"), xorstr_("F"));
	}
	else {
		yaw_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("y"), xorstr_("F"));
	}

	welllovely::instance->get_env()->DeleteLocalRef(player_class);

	welllovely::instance->get_env()->SetFloatField(player_obj, yaw_fid, yaw);
}

void c_player::add_yaw(float delta)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto yaw_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("rotationYaw") : BADLION_CLIENT ? xorstr_("y") : xorstr_("field_70177_z"), xorstr_("F"));
	auto prev_yaw_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("prevRotationYaw") : BADLION_CLIENT ? xorstr_("A") : xorstr_("field_70126_B"), xorstr_("F"));

	env->DeleteLocalRef(cls);

	if (!yaw_fid)
	{
		env->ExceptionClear();
		return;
	}

	env->SetFloatField(player_obj, yaw_fid, env->GetFloatField(player_obj, yaw_fid) + delta);

	if (prev_yaw_fid)
		env->SetFloatField(player_obj, prev_yaw_fid, env->GetFloatField(player_obj, prev_yaw_fid) + delta);
	else
		env->ExceptionClear();
}

float c_player::get_yaw()
{
	auto player_class = welllovely::instance->get_env()->GetObjectClass(player_obj);

	jfieldID yaw_fid;
	if (CUSTOM_CLIENT) {
		yaw_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("rotationYaw"), xorstr_("F"));
	}
	else if (!BADLION_CLIENT) {
		yaw_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70177_z"), xorstr_("F"));
	}
	else {
		yaw_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("y"), xorstr_("F"));
	}

	welllovely::instance->get_env()->DeleteLocalRef(player_class);

	return welllovely::instance->get_env()->GetFloatField(player_obj, yaw_fid);
}

void c_player::set_pitch(float pitch)
{
	auto player_class = welllovely::instance->get_env()->GetObjectClass(player_obj);

	jfieldID pitch_fid;
	if (CUSTOM_CLIENT) {
		pitch_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("rotationPitch"), xorstr_("F"));
	}
	else if (!BADLION_CLIENT) {
		pitch_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70125_A"), xorstr_("F"));
	}
	else {
		pitch_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("z"), xorstr_("F"));
	}

	welllovely::instance->get_env()->DeleteLocalRef(player_class);

	welllovely::instance->get_env()->SetFloatField(player_obj, pitch_fid, pitch);
}

float c_player::get_pitch()
{
	auto player_class = welllovely::instance->get_env()->GetObjectClass(player_obj);

	jfieldID pitch_fid;
	if (CUSTOM_CLIENT) {
		pitch_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("rotationPitch"), xorstr_("F"));
	}
	else if (!BADLION_CLIENT) {
		pitch_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70125_A"), xorstr_("F"));
	}
	else {
		pitch_fid = welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("z"), xorstr_("F"));
	}

	welllovely::instance->get_env()->DeleteLocalRef(player_class);

	return welllovely::instance->get_env()->GetFloatField(player_obj, pitch_fid);
}

std::shared_ptr<c_axisalignedbb> c_player::get_bounding_box()
{
	auto player_class = welllovely::instance->get_env()->GetObjectClass(player_obj);
	jfieldID boundingbox_fid;

	boundingbox_fid = CUSTOM_CLIENT ? welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("boundingBox"), xorstr_("Lnet/minecraft/util/AxisAlignedBB;")) : BADLION_CLIENT ? welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("f"), xorstr_("Laug;")) : welllovely::instance->get_env()->GetFieldID(player_class, xorstr_("field_70121_D"), xorstr_("Lnet/minecraft/util/AxisAlignedBB;"));

	welllovely::instance->get_env()->DeleteLocalRef(player_class);

	return std::make_shared<c_axisalignedbb>(welllovely::instance->get_env()->GetObjectField(player_obj, boundingbox_fid));
}

void c_player::set_bounding_box(s_axisalignedbb bb)
{
	auto env = welllovely::instance->get_env();

	auto aabb_cls = welllovely::instance->find_class_quiet(
		!BADLION_CLIENT ? xorstr_("net.minecraft.util.AxisAlignedBB") : xorstr_("aug"));
	if (!aabb_cls)
		return;

	auto ctor = env->GetMethodID(aabb_cls, xorstr_("<init>"), xorstr_("(DDDDDD)V"));
	if (!ctor)
	{
		env->ExceptionClear();
		env->DeleteLocalRef(aabb_cls);
		return;
	}

	auto new_bb = env->NewObject(aabb_cls, ctor,
		static_cast<jdouble>(bb.minX), static_cast<jdouble>(bb.minY), static_cast<jdouble>(bb.minZ),
		static_cast<jdouble>(bb.maxX), static_cast<jdouble>(bb.maxY), static_cast<jdouble>(bb.maxZ));
	env->DeleteLocalRef(aabb_cls);

	if (!new_bb)
	{
		env->ExceptionClear();
		return;
	}

	auto ent_cls = env->GetObjectClass(player_obj);
	auto bb_fid = env->GetFieldID(ent_cls,
		CUSTOM_CLIENT ? xorstr_("boundingBox") : BADLION_CLIENT ? xorstr_("f") : xorstr_("field_70121_D"),
		!BADLION_CLIENT ? xorstr_("Lnet/minecraft/util/AxisAlignedBB;") : xorstr_("Laug;"));
	env->DeleteLocalRef(ent_cls);

	if (bb_fid)
		env->SetObjectField(player_obj, bb_fid, new_bb);
	else
		env->ExceptionClear();

	env->DeleteLocalRef(new_bb);
}

double c_player::get_distance_to(std::shared_ptr<c_player> other)
{
	auto pos = get_position();
	auto entity_pos = other->get_position();
	return sdk::util::distance(pos.x, pos.y, pos.z, entity_pos.x, entity_pos.y, entity_pos.z);
}

void c_player::set_sprinting(bool value)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto set_sprint_mid = env->GetMethodID(cls,
		CUSTOM_CLIENT ? xorstr_("setSprinting") : BADLION_CLIENT ? xorstr_("d") : xorstr_("func_70031_b"), xorstr_("(Z)V"));

	env->DeleteLocalRef(cls);

	if (!set_sprint_mid)
	{
		env->ExceptionClear();
		return;
	}

	env->CallVoidMethod(player_obj, set_sprint_mid, static_cast<jboolean>(value));
}

void c_player::set_fall_distance(float v)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("fallDistance") : BADLION_CLIENT ? xorstr_("S") : xorstr_("field_70143_R"), xorstr_("F"));

	env->DeleteLocalRef(cls);

	if (!fid)
	{
		env->ExceptionClear();
		return;
	}

	env->SetFloatField(player_obj, fid, v);
}

float c_player::get_fall_distance()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto fid = env->GetFieldID(cls,
		CUSTOM_CLIENT ? xorstr_("fallDistance") : BADLION_CLIENT ? xorstr_("S") : xorstr_("field_70143_R"),
		xorstr_("F"));
	env->DeleteLocalRef(cls);

	if (!fid)
	{
		env->ExceptionClear();
		return 0.0f;
	}

	return env->GetFloatField(player_obj, fid);
}

void c_player::set_hurt_time(int v)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("hurtTime") : BADLION_CLIENT ? xorstr_("au") : xorstr_("field_70737_aN"), xorstr_("I"));

	env->DeleteLocalRef(cls);

	if (!fid)
	{
		env->ExceptionClear();
		return;
	}

	env->SetIntField(player_obj, fid, v);
}

void c_player::set_on_ground(bool v)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto fid = env->GetFieldID(cls,
		CUSTOM_CLIENT ? xorstr_("onGround") : BADLION_CLIENT ? xorstr_("C") : xorstr_("field_70122_E"),
		xorstr_("Z"));
	env->DeleteLocalRef(cls);
	if (!fid) { env->ExceptionClear(); return; }
	env->SetBooleanField(player_obj, fid, static_cast<jboolean>(v));
}

void c_player::set_sneaking(bool v)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto mid = env->GetMethodID(cls,
		CUSTOM_CLIENT ? xorstr_("setSneaking") : BADLION_CLIENT ? xorstr_("b") : xorstr_("func_70095_a"),
		xorstr_("(Z)V"));
	env->DeleteLocalRef(cls);
	if (!mid) { env->ExceptionClear(); return; }
	env->CallVoidMethod(player_obj, mid, static_cast<jboolean>(v));
}

void c_player::set_move_forward(float v)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto fid = env->GetFieldID(cls,
		CUSTOM_CLIENT ? xorstr_("moveForward") : BADLION_CLIENT ? xorstr_("ba") : xorstr_("field_70701_bs"),
		xorstr_("F"));
	env->DeleteLocalRef(cls);
	if (!fid) { env->ExceptionClear(); return; }
	env->SetFloatField(player_obj, fid, v);
}

void c_player::set_move_strafe(float v)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);
	auto fid = env->GetFieldID(cls,
		CUSTOM_CLIENT ? xorstr_("moveStrafing") : BADLION_CLIENT ? xorstr_("aZ") : xorstr_("field_70702_br"),
		xorstr_("F"));
	env->DeleteLocalRef(cls);
	if (!fid) { env->ExceptionClear(); return; }
	env->SetFloatField(player_obj, fid, v);
}

void c_player::set_position(sdk::vec3d pos)
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	jfieldID px, py, pz;
	if (CUSTOM_CLIENT) {
		px = env->GetFieldID(cls, xorstr_("posX"), xorstr_("D"));
		py = env->GetFieldID(cls, xorstr_("posY"), xorstr_("D"));
		pz = env->GetFieldID(cls, xorstr_("posZ"), xorstr_("D"));
	} else if (!BADLION_CLIENT) {
		px = env->GetFieldID(cls, xorstr_("field_70165_t"), xorstr_("D"));
		py = env->GetFieldID(cls, xorstr_("field_70163_u"), xorstr_("D"));
		pz = env->GetFieldID(cls, xorstr_("field_70161_v"), xorstr_("D"));
	} else {
		px = env->GetFieldID(cls, xorstr_("s"), xorstr_("D"));
		py = env->GetFieldID(cls, xorstr_("t"), xorstr_("D"));
		pz = env->GetFieldID(cls, xorstr_("u"), xorstr_("D"));
	}
	env->DeleteLocalRef(cls);

	if (!px || !py || !pz) { env->ExceptionClear(); return; }

	env->SetDoubleField(player_obj, px, pos.x);
	env->SetDoubleField(player_obj, py, pos.y);
	env->SetDoubleField(player_obj, pz, pos.z);
}

std::string c_player::get_name()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto mid = env->GetMethodID(cls, CUSTOM_CLIENT ? xorstr_("getName") : BADLION_CLIENT ? xorstr_("e_") : xorstr_("func_70005_c_"), xorstr_("()Ljava/lang/String;"));

	env->DeleteLocalRef(cls);

	if (!mid)
	{

		env->ExceptionClear();
		return std::string();
	}

	auto jname = static_cast<jstring>(env->CallObjectMethod(player_obj, mid));
	if (!jname)
	{
		env->ExceptionClear();
		return std::string();
	}

	const char* chars = env->GetStringUTFChars(jname, nullptr);
	std::string result = chars ? chars : "";
	if (chars)
		env->ReleaseStringUTFChars(jname, chars);
	env->DeleteLocalRef(jname);

	return result;
}

sdk::vec3d c_player::get_last_tick_pos()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(player_obj);

	auto x_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("lastTickPosX") : BADLION_CLIENT ? xorstr_("P") : xorstr_("field_70142_S"), xorstr_("D"));
	auto y_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("lastTickPosY") : BADLION_CLIENT ? xorstr_("Q") : xorstr_("field_70137_T"), xorstr_("D"));
	auto z_fid = env->GetFieldID(cls, CUSTOM_CLIENT ? xorstr_("lastTickPosZ") : BADLION_CLIENT ? xorstr_("R") : xorstr_("field_70136_U"), xorstr_("D"));

	env->DeleteLocalRef(cls);

	if (!x_fid || !y_fid || !z_fid)
	{
		env->ExceptionClear();
		return get_position();
	}

	return sdk::vec3d{
		env->GetDoubleField(player_obj, x_fid),
		env->GetDoubleField(player_obj, y_fid),
		env->GetDoubleField(player_obj, z_fid)
	};
}
