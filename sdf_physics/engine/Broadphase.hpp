#pragma once

#include <robin_hood/robin_hood.h>
#include <glm/vec2.hpp>
#include <entt/fwd.hpp>

class EntityManager;
class SystemManager;
class ShapeManager;
class Shape;
struct TransformComponent;

namespace std {
	template <>
	struct hash<glm::ivec2> {
		size_t operator()(const glm::ivec2& x) const{
			union {
				size_t hashed;
				glm::ivec2 v;
			};
			v = x;
			return hashed;
		}
	};
}

class Broadphase {
public:
	Broadphase(SystemManager&);

	void AddDynamic(entt::entity entity, const TransformComponent& transform, const Shape& shape);

	const std::vector<std::pair<entt::entity, entt::entity>>& GetPotentiallyIntersections();
private:
	struct Cell {
		std::vector<entt::entity> dynamic;
		//std::vector<entt::entity> static;
	};

	ShapeManager& m_shape_manager;
	EntityManager& m_entity_manager;

	robin_hood::unordered_node_map<glm::ivec2, Cell> m_cells;
	std::vector<std::pair<entt::entity, entt::entity>> m_intersections_cache;
};