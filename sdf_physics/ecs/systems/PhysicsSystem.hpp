#pragma once

#include <memory>
#include <robin_hood/robin_hood.h>
#include <glm/vec2.hpp>
#include "engine/System.hpp"
#include "ecs/EntityManager.hpp"
#include "engine/shape/ShapeId.hpp"

class SystemManager;
class ShapeManager;
class DebugDrawing;
class Broadphase;
class Input;

struct MassValues {
	// Relative to shape corner
	glm::vec2 center_of_mass{};
	float mass = 0.f;
	//float inertia;
};

struct Contact {
	entt::entity entity_left;
	entt::entity entity_right;
	glm::vec2 position;
	glm::vec2 normal;
	float intersection_depth;
	float previous_impulse;
	float previous_tangent_impulse;
};

class PhysicsSystem final : public System {
public:
	PhysicsSystem(SystemManager& system_manager);
	~PhysicsSystem();

private:
	void Initialize();
	void Update(float dt);

	MassValues& GetMassValues(const Shape& shape);

	EntityManager& m_entity_manager;
	ShapeManager& m_shape_manager;
	DebugDrawing& m_debug_drawing;
	Input& m_input;
	std::unique_ptr<Broadphase> m_broadphase;

	float m_update_timer = 0.f;

	entt::entity m_dragging_entity;

	std::vector<Contact> m_contacts;

	robin_hood::unordered_flat_map<ShapeId, MassValues> m_mass_values;
};
