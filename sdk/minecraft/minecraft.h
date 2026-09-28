#pragma once

#include "../includes.h"

namespace sdk {
	class c_minecraft {
	private:
	public:

		jobject get_minecraft();

		jobject get_player(jobject);

		jobject get_world(jobject mc);

		jobject get_current_screen(jobject mc);

		jobject get_entity_over(jobject mc);

		void click_mouse(jobject mc);

		void right_click_mouse(jobject mc);

		void right_click_release(jobject mc);

		void window_click(jobject mc, int slot, int mouse_btn, int mode);

		void send_open_inventory_packet(jobject mc);

		float get_partial_ticks(jobject mc);

		bool is_looking_at_block(jobject mc);

		float get_gamma(jobject mc);
		void  set_gamma(jobject mc, float value);

		void set_sneak_key_pressed(jobject mc, bool value);

		void set_use_key_pressed(jobject mc, bool value);

		bool get_move_input(jobject mc, float& forward, float& strafe);
	};

	extern std::unique_ptr<c_minecraft> instance;
}
