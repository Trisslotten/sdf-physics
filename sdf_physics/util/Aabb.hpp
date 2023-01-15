#pragma once

#include <glm/vec2.hpp>
#include <limits>

struct Aabb {
	glm::vec2 min = glm::vec2(std::numeric_limits<float>::max());
	glm::vec2 max = glm::vec2(std::numeric_limits<float>::lowest());
};