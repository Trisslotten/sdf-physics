#include "SystemManager.hpp"

SystemManager::SystemManager() {}

SystemManager::~SystemManager() {}

entt::sink<void()> SystemManager::OnInitialize() {
	return m_on_initialize;
}

entt::sink<void(float)> SystemManager::OnUpdate() {
	return m_on_update;
}

void SystemManager::Initialize() {
	m_on_initialize.publish();
}

void SystemManager::Update(float dt) {
	m_on_update.publish(dt);
}
