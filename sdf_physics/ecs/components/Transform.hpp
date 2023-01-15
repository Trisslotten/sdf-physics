#pragma once

#include <glm/vec2.hpp>
#include <glm/mat2x2.hpp>

struct TransformComponent {
	glm::vec2 position{};
	float rotation = 0;

	glm::mat2 CalculateRotationMatrix() const {
		float s = sin(rotation);
		float c = cos(rotation);
		return glm::mat2(c, s, -s, c);
	}
};
