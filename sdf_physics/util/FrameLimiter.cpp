#include "FrameLimiter.hpp"

#include <thread>

FrameLimiter::FrameLimiter()
	: m_frametime{ std::chrono::microseconds(static_cast<i64>(1'000'000.f/165.f)) }
{
	m_next_frame_start = std::chrono::steady_clock::now() + m_frametime;
}

void FrameLimiter::SetFramerate(float framerate) {
	m_frametime = std::chrono::microseconds(static_cast<i64>(1'000'000.f / framerate));

	m_next_frame_start = std::chrono::steady_clock::now() + m_frametime;
}

void FrameLimiter::Limit() {
	auto now = std::chrono::steady_clock::now();
	if (now < m_next_frame_start) {
		std::this_thread::sleep_until(m_next_frame_start);
	}
	m_next_frame_start += m_frametime;
}
