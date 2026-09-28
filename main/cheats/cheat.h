#pragma once

#include "../../sdk/includes.h"
class c_player;
class c_world;

class c_context {
public:
	std::shared_ptr<c_player> local;
	std::shared_ptr<c_world> world;
	bool hovering;
	bool ingame;

	jobject minecraft;

	c_context(std::shared_ptr<c_player> plr, std::shared_ptr<c_world> wld, bool ingam, bool hoverig, jobject mc)
	{
		local = plr;
		world = wld;
		ingame = ingam;
		hovering = hoverig;
		minecraft = mc;
	}
};
