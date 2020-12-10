#pragma once

#include "pbe/Core/UUID.h"
#include "pbe/Core/Ref.h"
#include "pbe/Core/Timestep.h"
#include "pbe/Core/Math/Common.h"
#include "pbe/Editor/EditorCamera.h"

#include "entt/entt.hpp"


namespace pbe {

	class Entity;
	using EntityMap = std::unordered_map<UUID, Entity>;

	class Scene : public RefCounted
	{
	public:
		Scene(const std::string& debugName = "Scene");
		~Scene();
		
		void Init();

		void OnUpdate(Timestep ts);

		// todo: tmp
		void OnRenderEntityInfo();
		void OnRenderScene(const Mat4& viewProj, const Vec3& camPos);
		void OnRenderRuntime();
		void OnRenderEditor(const EditorCamera& editorCamera);
		void OnEvent(Event& e);

		// Runtime
		void OnRuntimeStart();
		void OnRuntimeStop();

		void SetViewportSize(uint32_t width, uint32_t height);

		Entity GetMainCameraEntity();

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithID(UUID uuid, const std::string& name = "", bool runtimeMap = false);
		void DestroyEntity(Entity entity);

		Entity DuplicateEntity(Entity entity);

		template<typename T>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<T>();
		}

		Entity FindEntityByTag(const std::string& tag);

		const EntityMap& GetEntityMap() const { return m_EntityIDMap; }
		void CopyTo(Ref<Scene>& target);

		UUID GetUUID() const { return m_SceneID; }

		static Ref<Scene> GetScene(UUID uuid);

		// Editor-specific
		void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }
	private:
		UUID m_SceneID;
		entt::entity m_SceneEntity;
		entt::registry m_Registry;

		std::string m_DebugName;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		EntityMap m_EntityIDMap;

		entt::entity m_SelectedEntity;

		bool m_IsPlaying = false;

		void AddTransformComponent(Entity entity);
		
		friend class Entity;
		friend class SceneRenderer;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;

		friend void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity);
		friend void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity);
	};
}
