#pragma once

#include "Scene.h"

#include "Components.h"

namespace pbe {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene)
			: m_EntityHandle(handle), m_Scene(scene) {}

		~Entity() {}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			HZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		const T& GetComponent() const
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponentIfExist()
		{
			if (HasComponent<T>()) {
				RemoveComponent<T>();
			}
		}

		operator uint32_t () const { return (uint32_t)m_EntityHandle; }
		operator entt::entity () const { return m_EntityHandle; }
		operator bool () const { return (uint32_t)m_EntityHandle && m_Scene; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		UUID GetUUID() const { return GetComponent<IDComponent>().ID; }
		UUID GetSceneUUID() const { return m_Scene->GetUUID(); }
		Scene* GetScene() const { return m_Scene; }
	private:
		Entity(const std::string& name);
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

		friend class Scene;
		friend class SceneSerializer;
		friend class ScriptEngine;
	};

}
