#pragma once

#include <unordered_map>
#include <glm/vec2.hpp>

#include "engine/System.hpp"
#include "util/IntTypes.hpp"

struct GLFWwindow;

enum class ClientAction : u32 {
	MoveForward = 0,
	MoveBackward,
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	MoveFaster,

	DebugReloadShaders,

	MainDown,
};

enum class InputType {
	Down = 0,
	Pressed,
	Released,
};

struct Bind {
	i32 key = 0;
	InputType type = InputType::Down;

	bool IsKeyboardBind() const {
		return (((~0u) >> 16) & key) != 0;
	}
	i32 GetKeyboardValue() {
		return ((~0u) >> 16) & key;
	}
	i32 GetMouseValue() {
		return key >> 16;
	}

	Bind();
	Bind(i32 key) {
		this->key = key;
	}
	Bind(i32 key, InputType type) {
		this->key = key;
		this->type = type;
	}
};

class Input final : public System {
public:
	Input(GLFWwindow* glfw_window);

	void Update();

	bool Has(ClientAction) const;
	glm::vec2 GetMousePosition() const;
	glm::vec2 GetMouseMovement() const;

	glm::vec2 GetScroll() const;

	void SetMouseLocked(bool locked);

private:
	GLFWwindow* m_glfw_window;

	struct KeyState {
		i32 current_state = 0;
		i32 num_state_changes = 0;
	};

	std::vector<KeyState> m_key_states;
	std::vector<KeyState> m_mouse_states;
	std::unordered_map<ClientAction, Bind> m_keybinds;
	std::unordered_map<ClientAction, Bind> m_mousebinds;

	glm::dvec2 m_mouse_position{ 0, 0 };
	glm::vec2 m_last_mouse_position{ 0, 0 };
	glm::vec2 m_scroll{ 0, 0 };
	bool m_mouse_locked = false;
	bool m_mouse_was_locked = false;

	friend class InputCallbacks;
};
