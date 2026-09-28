#pragma once

#include "impl.h"
#include "../../module.h"

class c_teams : public c_module {
	static inline c_teams* s_instance = nullptr;

	static jmethodID get_team_mid(JNIEnv* env, jobject entity)
	{

		auto cls = env->GetObjectClass(entity);
		auto mid = env->GetMethodID(cls,
			CUSTOM_CLIENT ? xorstr_("getTeam") : BADLION_CLIENT ? xorstr_("bw") : xorstr_("func_152124_bt"),
			!BADLION_CLIENT ? xorstr_("()Lnet/minecraft/scoreboard/Team;") : xorstr_("Lew;"));
		env->DeleteLocalRef(cls);

		if (!mid)
			env->ExceptionClear();

		return mid;
	}

public:
	c_teams() : c_module(xorstr_("Teams"), e_category::utils)
	{
		s_instance = this;
	}

	static bool is_teammate(jobject target)
	{
		if (!s_instance || !s_instance->enabled || !target || !target)
			return false;

		auto env = welllovely::instance->get_env();
		if (!env)
			return false;

		auto mc = sdk::instance->get_minecraft();
		if (!mc)
			return false;

		auto lp = sdk::instance->get_player(mc);
		if (!lp)
		{
			env->DeleteLocalRef(mc);
			return false;
		}

		bool same = false;

		auto gt_mid = get_team_mid(env, lp);
		if (gt_mid)
		{
			jobject my_team = env->CallObjectMethod(lp, gt_mid);
			if (env->ExceptionCheck()) { env->ExceptionClear(); my_team = nullptr; }

			jobject tgt_team = env->CallObjectMethod(target, gt_mid);
			if (env->ExceptionCheck()) { env->ExceptionClear(); tgt_team = nullptr; }

			if (my_team && tgt_team)
			{

				auto team_cls = env->GetObjectClass(my_team);
				auto same_mid = env->GetMethodID(team_cls,
					CUSTOM_CLIENT ? xorstr_("isSameTeam") : BADLION_CLIENT ? xorstr_("a") : xorstr_("func_96665_a"),
					!BADLION_CLIENT ? xorstr_("(Lnet/minecraft/scoreboard/Team;)Z") : xorstr_("(Lew;)Z"));
				env->DeleteLocalRef(team_cls);

				if (same_mid)
					same = env->CallBooleanMethod(my_team, same_mid, tgt_team) != JNI_FALSE;
				else
					env->ExceptionClear();
			}

			if (my_team) env->DeleteLocalRef(my_team);
			if (tgt_team) env->DeleteLocalRef(tgt_team);
		}

		env->DeleteLocalRef(lp);
		env->DeleteLocalRef(mc);
		return same;
	}
};
