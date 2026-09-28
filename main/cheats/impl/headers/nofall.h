#pragma once

#include "impl.h"
#include "../../module.h"

class c_nofall : public c_module {
public:
	c_nofall() : c_module(xorstr_("NoFall"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
};
