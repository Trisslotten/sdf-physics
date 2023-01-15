#include "Engine.hpp"

#include "engine/SystemManager.hpp"
#include "engine/shape/ShapeManager.hpp"
#include "ecs/systems/PhysicsSystem.hpp"
#include "Window.hpp"
#include "Input.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/DebugDrawing.hpp"

Engine::Engine() {
	m_system_manager.Add<EntityManager>();
	m_system_manager.Add<DebugDrawing>();
	m_system_manager.Add<ShapeManager>();
	m_system_manager.Add<PhysicsSystem>(m_system_manager);

	m_system_manager.Add<Window>(m_system_manager, 1280, 720);
	m_system_manager.Add<Graphics::Renderer>(m_system_manager);

	//m_frame_limiter.SetFramerate(10.f);
}

bool Engine::IsRunning() const {
	return m_running && !m_system_manager.Get<Window>().ShouldClose();
}

void Engine::StopRunning() {
	m_running = false;
}

void Engine::Initialize() {
	srand(time(nullptr));

	m_system_manager.Initialize();
}

void Engine::Update(float deltatime) {
	{
		static Camera camera;
		auto& renderer = m_system_manager.Get<Graphics::Renderer>();
		auto& input = m_system_manager.Get<Input>();
		auto& window = m_system_manager.Get<Window>();

		glm::vec2 window_size = glm::vec2(window.GetWindowSize());

		if (input.Has(ClientAction::MainDown)) {
			camera.position -= camera.zoom * input.GetMouseMovement() / window_size.y;
		}

		float zoom_change = 1.0f + 0.1f * input.GetScroll().y;
		camera.zoom *= zoom_change;

		auto center_offset = input.GetMousePosition() - window_size/2.f;
		glm::vec2 normalized_center_offset = 2.0f * center_offset / window_size.x;

		camera.position -= normalized_center_offset * (zoom_change-1.0f) * camera.zoom;

		renderer.SetCamera(camera);
	}

	m_system_manager.Update(deltatime);

	//m_frame_limiter.Limit();
}
