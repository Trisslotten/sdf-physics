#pragma once

#include <chrono>

#include "IntTypes.hpp"

class FrameLimiter {
public:
	FrameLimiter();

	void SetFramerate(float framerate);
	void Limit();
private:
	std::chrono::steady_clock::time_point m_next_frame_start;
	std::chrono::microseconds m_frametime;
};
