#pragma once

#include "impl.h"
#include "../../module.h"

class c_sprint : public c_module {
public:
	c_sprint() : c_module(xorstr_("Sprint"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
};
