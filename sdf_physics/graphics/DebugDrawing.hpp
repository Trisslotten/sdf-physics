#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include "engine/System.hpp"

struct DebugLine {
	glm::vec2 start;
	glm::vec2 end;
};

class DebugDrawing final : public System {
public:
	DebugDrawing();
	~DebugDrawing();

	void AddLine(glm::vec2 start, glm::vec2 end);
	void AddVector(glm::vec2 tip, glm::vec2 end);

	void AddCross(glm::vec2 pos, float size);

	const std::vector<DebugLine>& GetLines() const;

	void Clear();
private:
	std::vector<DebugLine> m_lines;
};