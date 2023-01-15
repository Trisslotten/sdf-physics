#include "EntityManager.hpp"

EntityManager::EntityManager() {
	m_invalid_entity = create();
}

EntityManager::~EntityManager() {}

entt::entity EntityManager::invalid_entity() const {
	return m_invalid_entity;
}
