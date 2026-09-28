#pragma once

#include "impl.h"
#include "../../module.h"

class c_strafe : public c_module {
public:
	c_strafe() : c_module(xorstr_("Strafe"), e_category::movement, 0) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;

private:
	c_timer m_timer;
};
