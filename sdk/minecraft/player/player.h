#pragma once

#include "../minecraft.h"

class c_axisalignedbb;
struct s_axisalignedbb;

class c_player {
private:
	jobject player_obj;
public:
	c_player(jobject obj);
	~c_player();

	bool is_invisible();
	bool is_sprinting();

	float get_ai_move_speed();
	float get_hurt_time();
	float get_moveforward();
	float get_movestrafe();
	sdk::vec3d get_motion_vector();
	void get_motion_vector(sdk::vec3d vec);
	bool is_on_ground();
	float get_health();
	sdk::vec3d get_position();
	void set_yaw(float yaw);
	void add_yaw(float delta);
	float get_yaw();
	void set_pitch(float pitch);
	float get_pitch();
	std::shared_ptr<c_axisalignedbb> get_bounding_box();
	void set_bounding_box(s_axisalignedbb bb);
	double get_distance_to(std::shared_ptr<c_player>);
	void set_sprinting(bool value);
	sdk::vec3d get_last_tick_pos();
	void set_fall_distance(float v);
	float get_fall_distance();
	void set_hurt_time(int v);
	void set_on_ground(bool v);
	void set_sneaking(bool v);
	void set_move_forward(float v);
	void set_move_strafe(float v);
	void set_position(sdk::vec3d pos);
	std::string get_name();

	const jobject get_object()
	{
		return player_obj;
	}
};
