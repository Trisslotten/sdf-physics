#pragma once

#include "engine/System.hpp"
#include <glad/glad.h>
#include <functional>
#include <glm/vec2.hpp>

#include "util/IntTypes.hpp"

struct GLFWwindow;
class Input;
class SystemManager;

class Window final : public System {
public:
	using ResizeCallback = std::function<void(i32 width, i32 height)>;

	Window(SystemManager& system_manager, i32 width, i32 height);
	~Window();
	void Update(float dt);
	bool ShouldClose() const;

	void RegisterResizedCallback(ResizeCallback callback);

	glm::ivec2 GetWindowSize() const;

private:
	GLFWwindow* m_glfw_window = nullptr;
	Input* m_input = nullptr;

	std::vector<ResizeCallback> m_resized_callbacks;

	friend class Input;
	friend class WindowCallbacks;
	friend class InputCallbacks;
};
