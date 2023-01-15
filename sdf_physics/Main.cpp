#include "engine/Engine.hpp"
#include "window.hpp"
#include "graphics/renderer.hpp"

int main() {
	Engine engine;

	engine.Initialize();

	auto first_frame_dt = std::chrono::milliseconds(16);
	auto previous_time = std::chrono::steady_clock::now() - first_frame_dt;
	while (engine.IsRunning()) {
		auto now = std::chrono::steady_clock::now();
		auto nano_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - previous_time).count();
		float deltatime = static_cast<float>(nano_diff) / static_cast<float>(std::nano::den);

		engine.Update(deltatime);

		previous_time = now;
	}
	return 0;
}