#pragma once

#include <entt/entity/registry.hpp>
#include "engine/System.hpp"

class EntityManager : public entt::registry, public System {
public:
	EntityManager();
	~EntityManager();

	entt::entity invalid_entity() const;
private:
	entt::entity m_invalid_entity;
};