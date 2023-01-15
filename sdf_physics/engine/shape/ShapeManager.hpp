#pragma once

#include <robin_hood/robin_hood.h>
#include "engine/System.hpp"
#include "ShapeId.hpp"
#include "Shape.hpp"

class ShapeManager final : public System {
public:
	Shape& CreateShape();
	Shape& CreateShape(Shape shape);

	bool DeleteShape(ShapeId id);

	Shape* GetShape(ShapeId id);
	const Shape* GetShape(ShapeId id) const;

	bool HasShape(ShapeId id) const;
private:
	robin_hood::unordered_map<ShapeId, Shape> m_shapes;
	TypeSafeIdGenerator<ShapeId> m_id_generator;
};