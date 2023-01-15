#include "ShapeManager.hpp"

Shape& ShapeManager::CreateShape() {
	Shape shape;
	return CreateShape(std::move(shape));
}

Shape& ShapeManager::CreateShape(Shape shape) {
	auto new_id = m_id_generator.Generate();
	auto iter = m_shapes.emplace(new_id, std::move(shape));
	iter.first->second.SetId(new_id);
	return iter.first->second;
}

bool ShapeManager::DeleteShape(ShapeId id) {
	return m_shapes.erase(id) != 0;
}

Shape* ShapeManager::GetShape(ShapeId id) {
	if (auto iter = m_shapes.find(id); iter != m_shapes.end()) {
		return &iter->second;
	}
	return nullptr;
}

const Shape* ShapeManager::GetShape(ShapeId id) const {
	if (auto iter = m_shapes.find(id); iter != m_shapes.end()) {
		return &iter->second;
	}
	return nullptr;
}

bool ShapeManager::HasShape(ShapeId id) const {
	return m_shapes.contains(id);
}
