#pragma once

#include <entt/signal/sigh.hpp>
#include <entt/core/type_info.hpp>
#include <robin_hood/robin_hood.h>

#include "util/IntTypes.hpp"
#include "System.hpp"

#include <iostream>

class SystemManager {
public:
	SystemManager();
	~SystemManager();

	entt::sink<void()> OnInitialize();
	entt::sink<void(float)> OnUpdate();

	template <class TSystem, class...Args>
	TSystem& Add(Args&&...args) {
		constexpr u32 id = entt::internal::type_hash<TSystem>((int)0);
		auto [iter, addedNew] = m_systems.emplace(id, std::make_unique<TSystem>(std::forward<Args>(args)...));
		if (!addedNew) {
			std::cout << "ERROR: added duplicate system\n";
			__debugbreak();
		}
		return *static_cast<TSystem*>(iter->second.get());
	}

	template <class TSystem>
	bool Has() {
		constexpr u32 id = entt::internal::type_hash<TSystem>((int)0);
		auto iter = m_systems.find(id);
		return iter != m_systems.end();
	}

	template <class TSystem>
	TSystem& Get() const {
		TSystem* result = nullptr;
		constexpr u32 id = entt::internal::type_hash<TSystem>((int)0);
		auto iter = m_systems.find(id);
		if (iter != m_systems.end()) {
			result = static_cast<TSystem*>(iter->second.get());
		}
		return *result;
	}

	void Initialize();
	void Update(float dt);

private:
	robin_hood::unordered_flat_map<u32, std::unique_ptr<System>> m_systems;

	entt::sigh<void()> m_on_initialize;
	entt::sigh<void(float)> m_on_update;
};
