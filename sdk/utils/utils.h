#pragma once

namespace sdk {

	struct vec3d {
		double x, y, z;
	};

	using qword = unsigned long long;
	using t_createdvms = jint(__stdcall*)(JavaVM**, jsize, jsize*);

	namespace util {
		inline float wrap_to_180(float value)
		{
			if (value >= 180.f)
				value -= 360.f;
			if (value < -180.f)
				value += 360.f;
			return value;
		}

		inline float radiants_to_deg(float x)
		{
			return x * 180.f / M_PI;
		}

		inline float deg_to_radiants(float r)
		{
			return r * M_PI / 180;
		}

		inline std::pair<float, float> get_angles(sdk::vec3d pos, sdk::vec3d pos1)
		{
			double d_x = pos1.x - pos.x;
			double d_y = pos1.y - pos.y;
			double d_z = pos1.z - pos.z;

			double hypothenuse = sqrt(d_x * d_x + d_z * d_z);
			float yaw = radiants_to_deg(atan2(d_z, d_x)) - 90.f;
			float pitch = radiants_to_deg(-atan2(d_y, hypothenuse));

			return std::make_pair(yaw, pitch);
		}

		inline static double distance(double x, double y) {
			return sqrt(pow(x, 2) + pow(y, 2));
		}

		inline static double distance(double x1, double y1, double z1, double x2, double y2, double z2) {
			return distance(y1 - y2, distance(x1 - x2, z1 - z2));
		}

		inline bool world_to_screen(const sdk::vec3d& rel, const float mv[16], const float proj[16],
			const int vp[4], float& out_x, float& out_y)
		{
			const double x = rel.x, y = rel.y, z = rel.z;

			float ex = (float)(mv[0] * x + mv[4] * y + mv[8]  * z + mv[12]);
			float ey = (float)(mv[1] * x + mv[5] * y + mv[9]  * z + mv[13]);
			float ez = (float)(mv[2] * x + mv[6] * y + mv[10] * z + mv[14]);
			float ew = (float)(mv[3] * x + mv[7] * y + mv[11] * z + mv[15]);

			float cx = proj[0] * ex + proj[4] * ey + proj[8]  * ez + proj[12] * ew;
			float cy = proj[1] * ex + proj[5] * ey + proj[9]  * ez + proj[13] * ew;
			float cw = proj[3] * ex + proj[7] * ey + proj[11] * ez + proj[15] * ew;

			if (cw < 0.001f)
				return false;

			float ndc_x = cx / cw;
			float ndc_y = cy / cw;

			out_x = vp[0] + (ndc_x * 0.5f + 0.5f) * vp[2];
			out_y = vp[1] + (1.0f - (ndc_y * 0.5f + 0.5f)) * vp[3];
			return true;
		}
	}
};
