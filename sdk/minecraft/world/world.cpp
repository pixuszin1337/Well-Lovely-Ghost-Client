#include "world.h"

#include "../../../main/welllovely.h"

extern jvmtiEnv* jvmti_env;

c_world::c_world(jobject obj)
{
	world_obj = obj;
}

c_world::~c_world()
{
	welllovely::instance->get_env()->DeleteLocalRef(world_obj);
}

std::vector<std::shared_ptr<c_player>> c_world::get_players()
{
	auto env = welllovely::instance->get_env();
	auto cls = env->GetObjectClass(world_obj);

	jfieldID player_entities;
	if (CUSTOM_CLIENT)
		player_entities = env->GetFieldID(cls, xorstr_("playerEntities"), xorstr_("Ljava/util/List;"));
	else if (!BADLION_CLIENT)
		player_entities = env->GetFieldID(cls, xorstr_("field_73010_i"), xorstr_("Ljava/util/List;"));
	else
		player_entities = env->GetFieldID(cls, xorstr_("j"), xorstr_("Ljava/util/List;"));

	env->DeleteLocalRef(cls);

	jclass list_cls = env->FindClass(xorstr_("java/util/List"));
	jmethodID to_array_md = env->GetMethodID(list_cls, xorstr_("toArray"), xorstr_("()[Ljava/lang/Object;"));
	env->DeleteLocalRef(list_cls);

	std::vector<std::shared_ptr<c_player>> res;

	jobject obj_player_entities = env->GetObjectField(world_obj, player_entities);
	if (!obj_player_entities)
		return res;

	auto array_player_list = reinterpret_cast<jobjectArray>(env->CallObjectMethod(obj_player_entities, to_array_md));
	env->DeleteLocalRef(obj_player_entities);

	if (!array_player_list)
		return res;

	size_t len = env->GetArrayLength(array_player_list);

	for (int i = 0; i < len; ++i)
	{
		jobject player = env->GetObjectArrayElement(array_player_list, i);
		res.push_back(std::make_shared<c_player>(player));
	}

	env->DeleteLocalRef(array_player_list);

	return res;
}

static jclass    s_bp_cls   = nullptr;
static jmethodID s_bp_ctor  = nullptr;
static jmethodID s_air_mid  = nullptr;
static bool      s_resolved = false;

static jmethodID find_air_method(JNIEnv* env, jclass world_cls, const std::string& desc)
{
	jclass cur = (jclass)env->NewLocalRef(world_cls);
	jmethodID result = nullptr;

	while (cur && !result)
	{
		jint mc = 0;
		jmethodID* mds = nullptr;
		jvmti_env->GetClassMethods(cur, &mc, &mds);

		for (jint i = 0; i < mc && !result; i++)
		{
			char* mn = nullptr; char* ms = nullptr;
			jvmti_env->GetMethodName(mds[i], &mn, &ms, nullptr);

			if (ms && desc == ms)
				result = mds[i];

			if (mn) jvmti_env->Deallocate((unsigned char*)mn);
			if (ms) jvmti_env->Deallocate((unsigned char*)ms);
		}
		if (mds) jvmti_env->Deallocate((unsigned char*)mds);

		jclass parent = env->GetSuperclass(cur);
		env->DeleteLocalRef(cur);
		cur = parent;
	}
	if (cur) env->DeleteLocalRef(cur);
	return result;
}

bool c_world::is_air_block(int x, int y, int z)
{
	auto env = welllovely::instance->get_env();
	if (!env || !world_obj) return false;

	if (!s_resolved)
	{
		s_resolved = true;

		jclass local_bp = nullptr;
		if (BADLION_CLIENT)
		{
			local_bp = welllovely::instance->find_class_quiet(xorstr_("cj"));
			if (!local_bp) local_bp = welllovely::instance->find_class_quiet(xorstr_("dt"));
		}
		else
			local_bp = welllovely::instance->find_class_quiet(xorstr_("net.minecraft.util.BlockPos"));

		if (!local_bp) return false;

		s_bp_cls = (jclass)env->NewGlobalRef(local_bp);
		env->DeleteLocalRef(local_bp);

		s_bp_ctor = env->GetMethodID(s_bp_cls, xorstr_("<init>"), xorstr_("(III)V"));
		if (!s_bp_ctor) { env->ExceptionClear(); return false; }

		char* bp_sig = nullptr;
		jvmti_env->GetClassSignature(s_bp_cls, &bp_sig, nullptr);
		if (!bp_sig) return false;

		std::string desc = std::string("(") + bp_sig + ")Z";
		jvmti_env->Deallocate((unsigned char*)bp_sig);

		auto world_cls = env->GetObjectClass(world_obj);

		if (CUSTOM_CLIENT)
			s_air_mid = env->GetMethodID(world_cls, xorstr_("isAirBlock"), desc.c_str());
		else if (!BADLION_CLIENT)
			s_air_mid = env->GetMethodID(world_cls, xorstr_("func_175623_d"), desc.c_str());

		if (!s_air_mid)
		{
			if (env->ExceptionCheck()) env->ExceptionClear();
			s_air_mid = find_air_method(env, world_cls, desc);
		}

		env->DeleteLocalRef(world_cls);
		if (env->ExceptionCheck()) env->ExceptionClear();
	}

	if (!s_bp_cls || !s_bp_ctor || !s_air_mid) return false;

	jobject bp = env->NewObject(s_bp_cls, s_bp_ctor, (jint)x, (jint)y, (jint)z);
	if (!bp) { env->ExceptionClear(); return false; }

	jboolean result = env->CallBooleanMethod(world_obj, s_air_mid, bp);
	if (env->ExceptionCheck()) { env->ExceptionClear(); result = JNI_FALSE; }

	env->DeleteLocalRef(bp);
	return (bool)result;
}
