#pragma once

#include "impl.h"
#include "../../module.h"

class c_noslowdown : public c_module {
	double m_written_x = 0.0;
	double m_written_z = 0.0;
	bool m_did_write = false;

	jfieldID m_fwd_fid = nullptr;
	jfieldID m_str_fid = nullptr;
	bool m_resolved = false;

public:
	c_noslowdown() : c_module(xorstr_("No Slowdown"), e_category::movement) {}

	void on_tick(std::shared_ptr<c_context> ctx) override;
};
