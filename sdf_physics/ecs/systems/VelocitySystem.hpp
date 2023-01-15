#pragma once

#include "engine/System.hpp"
#include "engine/SystemManager.hpp"
#include "ecs/EntityManager.hpp"

#include "ecs/components/Transform.hpp"
#include "ecs/components/Velocity.hpp"

#include <iostream>

class VelocitySystem : public System {
public:
	VelocitySystem(SystemManager& system_manager)
		: m_entity_manager{ system_manager.Get<EntityManager>() }
	{
		system_manager.OnUpdate().connect<&VelocitySystem::Update>(this);
	}
private:
	void Update(float dt) {
		for (auto &&[entity, transform, velocity] : m_entity_manager.view<TransformComponent, Velocity>().each()) {
			transform.position += velocity.velocity * dt;
			transform.rotation += velocity.angular_velocity * dt;
		}
	}

	EntityManager& m_entity_manager;
};
