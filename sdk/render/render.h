#pragma once

#include "../includes.h"

namespace sdk {

	struct s_view {
		float modelview[16];
		float projection[16];
		int   viewport[4];
		vec3d render_pos;
		bool  valid;
		int   stage;
	};

	class c_render {
	public:

		s_view capture();
	};

	struct s_box2d {
		bool  box_valid;
		float min_x, min_y, max_x, max_y;
		bool  feet_valid; float feet_x, feet_y;
		bool  head_valid; float head_x, head_y;
	};

	s_box2d project_box(const s_view& view, const sdk::vec3d& feet, double width, double height);

#ifdef _DEBUG
	void debug_dump_fields(const char* class_name);
	void debug_probe_method(const char* class_name, const char* method_name, const char* sig);
#endif
}
