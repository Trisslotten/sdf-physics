#include "Broadphase.hpp"

#include "SystemManager.hpp"
#include "shape/ShapeManager.hpp"
#include "ecs/EntityManager.hpp"
#include "ecs/components/Transform.hpp"
#include "util/Aabb.hpp"
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

constexpr float c_CellSize = 8.0f;

glm::ivec2 ToCellSpace(glm::vec2 pos) {
	return glm::ivec2(glm::floor(pos / c_CellSize));
}

Broadphase::Broadphase(SystemManager& system_manager)
	: m_shape_manager{ system_manager.Get<ShapeManager>() }
	, m_entity_manager{ system_manager.Get<EntityManager>() }
{

}

void Broadphase::AddDynamic(entt::entity entity, const TransformComponent& transform, const Shape& shape) {
	auto size = shape.GetSizeInMeters();
	float radius = 0.5f * glm::length(size);
	glm::vec2 min = transform.position - glm::vec2(radius);
	glm::vec2 max = transform.position + glm::vec2(radius);

	glm::ivec2 start = ToCellSpace(min);
	glm::ivec2 end = ToCellSpace(max);
	glm::ivec2 index;
	for (index.y = start.y; index.y <= end.y; ++index.y) {
		for (index.x = start.x; index.x <= end.x; ++index.x) {
			auto [iter, added_new] = m_cells.try_emplace(index);
			iter->second.dynamic.push_back(entity);
		}
	}
}

const std::vector<std::pair<entt::entity, entt::entity>>& Broadphase::GetPotentiallyIntersections() {
	m_intersections_cache.clear();

	for (auto& [key, cell] : m_cells) {
		auto& dynamic = cell.dynamic;
		for (auto iter_left = dynamic.begin(); iter_left != dynamic.end(); ++iter_left) {
			auto entity_left = *iter_left;
			const auto& [transform_left, shape_id_left] = m_entity_manager.get<TransformComponent, ShapeId>(entity_left);
			auto& shape_left = *m_shape_manager.GetShape(shape_id_left);

			auto size_left = shape_left.GetSizeInMeters();
			float radius_left = 0.5f * glm::length(size_left);

			for (auto iter_right = iter_left + 1; iter_right != dynamic.end(); ++iter_right) {
				auto entity_right = *iter_right;
				const auto& [transform_right, shape_id_right] = m_entity_manager.get<TransformComponent, ShapeId>(entity_right);
				auto& shape_right = *m_shape_manager.GetShape(shape_id_right);

				auto size_right = shape_right.GetSizeInMeters();
				float radius_right = 0.5f * glm::length(size_right);

				float radius_sum = radius_left + radius_right;
				float distance2 = glm::length2(transform_left.position - transform_right.position);
				if (distance2 <= radius_sum * radius_sum) {
					if (entity_left < entity_right) {
						m_intersections_cache.push_back({ entity_left, entity_right });
					} else {
						m_intersections_cache.push_back({ entity_right, entity_left });
					}
				}
			}
		}

		cell.dynamic.clear();
	}

	std::sort(m_intersections_cache.begin(), m_intersections_cache.end());
	auto unique_iter = std::unique(m_intersections_cache.begin(), m_intersections_cache.end());
	m_intersections_cache.erase(unique_iter, m_intersections_cache.end());

	return m_intersections_cache;
}
