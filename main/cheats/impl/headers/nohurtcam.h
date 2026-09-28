#pragma once

#include "impl.h"
#include "../../module.h"

class c_nohurtcam : public c_module {
public:
	c_nohurtcam() : c_module(xorstr_("No Hurt Cam"), e_category::utils) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
};
