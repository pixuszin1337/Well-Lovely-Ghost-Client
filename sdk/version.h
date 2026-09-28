#pragma once

namespace sdk {

	enum class e_version { unknown, v1_7_10, v1_8_9 };

	inline e_version version = e_version::unknown;

	void detect_version();

	inline const char* pick(const char* v1_7_10, const char* v1_8_9)
	{
		return version == e_version::v1_8_9 ? v1_8_9 : v1_7_10;
	}

	inline const char* version_string()
	{
		switch (version)
		{
		case e_version::v1_7_10: return "1.7.10";
		case e_version::v1_8_9:  return "1.8.9";
		default:                 return "?";
		}
	}
}
