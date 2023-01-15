#pragma once

#include "SystemManager.hpp"
#include "util/FrameLimiter.hpp"

class Engine {
public:
	Engine();

	bool IsRunning() const;
	void StopRunning();

	void Initialize();
	void Update(float deltatime);

private:
	SystemManager m_system_manager;
	FrameLimiter m_frame_limiter;
	std::atomic_bool m_running = true;
};