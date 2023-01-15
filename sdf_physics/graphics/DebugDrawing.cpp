#include "DebugDrawing.hpp"

DebugDrawing::DebugDrawing() {}

DebugDrawing::~DebugDrawing() {}

void DebugDrawing::AddLine(glm::vec2 start, glm::vec2 end) {
	m_lines.push_back({ start, end });
}

void DebugDrawing::AddCross(glm::vec2 pos, float size) {
	AddLine(pos + glm::vec2(size, 0), pos - glm::vec2(size, 0));
	AddLine(pos + glm::vec2(0, size), pos - glm::vec2(0, size));
}

const std::vector<DebugLine>& DebugDrawing::GetLines() const {
	return m_lines;
}

void DebugDrawing::Clear() {
	m_lines.clear();
}
