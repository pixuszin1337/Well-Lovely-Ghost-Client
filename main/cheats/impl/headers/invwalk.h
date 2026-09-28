#pragma once

#include "impl.h"
#include "../../module.h"

class c_invwalk : public c_module {
public:
	c_invwalk() : c_module(xorstr_("Inv Walk"), e_category::utils) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
};
