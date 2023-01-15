#include "input.hpp"

#include "window.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <optional>

class InputCallbacks {
public:
	static void KeyCallback(GLFWwindow* glfw_window, i32 key, i32 scancode, i32 action, i32 mods) {
		Input* input = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window))->m_input;

		if (key != GLFW_KEY_UNKNOWN && action != GLFW_REPEAT) {
			auto& key_state = input->m_key_states[key];
			key_state.current_state = action;
			key_state.num_state_changes++;
		}
	}
	static void MouseCallback(GLFWwindow* glfw_window, i32 button, i32 action, i32 mods) {
		Input* input = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window))->m_input;

		if (button != GLFW_KEY_UNKNOWN && action != GLFW_REPEAT) {
			auto& key_state = input->m_mouse_states[button];
			key_state.current_state = action;
			key_state.num_state_changes++;
		}
	}
	static void ScrollCallback(GLFWwindow* glfw_window, double x, double y) {
		Input* input = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window))->m_input;
		input->m_scroll += glm::vec2(x, y);
	}

private:
	InputCallbacks() = default;
};

Bind::Bind() : key{ GLFW_KEY_UNKNOWN }, type{ InputType::Down } {}

Input::Input(GLFWwindow* glfw_window) : m_glfw_window{ glfw_window } {
	glfwSetKeyCallback(glfw_window, InputCallbacks::KeyCallback);
	glfwSetMouseButtonCallback(glfw_window, InputCallbacks::MouseCallback);
	glfwSetScrollCallback(glfw_window, InputCallbacks::ScrollCallback);

	m_keybinds[ClientAction::MoveForward] = GLFW_KEY_W;
	m_keybinds[ClientAction::MoveBackward] = GLFW_KEY_S;
	m_keybinds[ClientAction::MoveLeft] = GLFW_KEY_A;
	m_keybinds[ClientAction::MoveRight] = GLFW_KEY_D;
	m_keybinds[ClientAction::MoveUp] = GLFW_KEY_SPACE;
	m_keybinds[ClientAction::MoveDown] = GLFW_KEY_LEFT_CONTROL;
	m_keybinds[ClientAction::MoveFaster] = GLFW_KEY_LEFT_SHIFT;

	m_keybinds[ClientAction::DebugReloadShaders] = { GLFW_KEY_F5, InputType::Released };

	m_mousebinds[ClientAction::MainDown] = GLFW_MOUSE_BUTTON_RIGHT;

	m_key_states.resize(GLFW_KEY_LAST);
	m_mouse_states.resize(GLFW_MOUSE_BUTTON_LAST);
}

glm::vec2 Input::GetMousePosition() const {
   return glm::vec2(m_mouse_position);
}

glm::vec2 Input::GetMouseMovement() const {
   return GetMousePosition() - m_last_mouse_position;
}

glm::vec2 Input::GetScroll() const {
	return m_scroll;
}

void Input::SetMouseLocked(bool locked) {
   m_mouse_locked = locked;
   if (locked) {
      glfwSetInputMode(m_glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   } else {
      glfwSetInputMode(m_glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
   }
}

void Input::Update() {
	glm::ivec2 windowSize;
	glfwGetWindowSize(m_glfw_window, &windowSize.x, &windowSize.y);

	m_last_mouse_position = m_mouse_position;
	glfwGetCursorPos(m_glfw_window, &m_mouse_position.x, &m_mouse_position.y);
	m_mouse_position.y = windowSize.y - m_mouse_position.y;

	m_scroll = { 0, 0 };

	for (auto& key_state : m_key_states) {
		key_state.num_state_changes = 0;
	}
	for (auto& key_state : m_mouse_states) {
		key_state.num_state_changes = 0;
	}
}

bool Input::Has(ClientAction action) const {
	std::optional<KeyState> state;
	std::optional<Bind> bind;

	{
		auto iter = m_keybinds.find(action);
		if (iter != m_keybinds.end() && iter->second.key != GLFW_KEY_UNKNOWN) {
			bind = iter->second;
			state = m_key_states[iter->second.key];
		}
	}
	{
		auto iter = m_mousebinds.find(action);
		if (iter != m_mousebinds.end()) {
			bind = iter->second;
			state = m_mouse_states[iter->second.key];
		}
	}

	if (state && bind) {
		auto& keybind = bind.value();
		auto& key_state = state.value();

		switch (keybind.type) {
		case InputType::Down:
			return key_state.current_state == GLFW_PRESS || key_state.num_state_changes >= 2;
		case InputType::Pressed:
			return key_state.current_state == GLFW_PRESS && key_state.num_state_changes >= 1 || key_state.num_state_changes >= 2;
		case InputType::Released:
			return key_state.current_state == GLFW_RELEASE && key_state.num_state_changes >= 1;
		default:
			std::cerr << "ERROR: invalid input type" << std::endl;
			return false;
		}
	}

	return false;
}
