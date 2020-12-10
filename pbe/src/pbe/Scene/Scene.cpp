#include "pch.h"
#include "Scene.h"

#include "Entity.h"

#include "Components.h"

#include "pbe/Renderer/SceneRenderer.h"
#include "pbe/Script/ScriptEngine.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pbe/Renderer/RendPrim.h"


namespace pbe {

	static const std::string DefaultEntityName = "Entity";

	std::unordered_map<UUID, Scene*> s_ActiveScenes;

	struct SceneComponent
	{
		UUID SceneID;
	};

	static void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
	{
		auto sceneView = registry.view<SceneComponent>();
		UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;

		Scene* scene = s_ActiveScenes[sceneID];

		auto entityID = registry.get<IDComponent>(entity).ID;
		HZ_CORE_ASSERT(scene->m_EntityIDMap.find(entityID) != scene->m_EntityIDMap.end());
		s_ScriptEngine->InitScriptEntity(scene->m_EntityIDMap.at(entityID));
	}

	static void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity)
	{
		auto sceneView = registry.view<SceneComponent>();
		UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;

		Scene* scene = s_ActiveScenes[sceneID];

		auto entityID = registry.get<IDComponent>(entity).ID;
		s_ScriptEngine->ShutdownScriptEntity(Entity{ entity, scene });
	}

	Scene::Scene(const std::string& debugName)
		: m_DebugName(debugName), m_SceneID(UUIDGet())
	{
		m_Registry.on_construct<ScriptComponent>().connect<&OnScriptComponentConstruct>();
		m_Registry.on_destroy<ScriptComponent>().connect<&OnScriptComponentDestroy>();

		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

		s_ActiveScenes[m_SceneID] = this;

		Init();
	}

	Scene::~Scene()
	{
		DestroyAllEntities();

		m_Registry.on_destroy<ScriptComponent>().disconnect();

		UUIDFree(m_SceneID);

		m_Registry.clear();
		s_ActiveScenes.erase(m_SceneID);
		s_ScriptEngine->ShutdownScene(this);
	}

	void Scene::Init()
	{
		s_ScriptEngine->InitScene(this);
	}

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	void Scene::OnUpdate(Timestep ts)
	{
		// Update all entities
		{
			auto view = m_Registry.view<ScriptComponent>();
			for (auto entity : view)
			{
				UUID entityID = m_Registry.get<IDComponent>(entity).ID;
				s_ScriptEngine->OnUpdateEntity(Entity{ entity, this }, ts);
			}
		}
	}

	void Scene::OnRenderEntityInfo()
	{
		Color pickColor = Color{ 15 / 255.f, 133 / 255.f, 212 / 255.f, 255 };
		float pickRadius = 0.2f;

		m_Registry.view<TransformComponent, DirectionLightComponent>().each([&](TransformComponent& trans, DirectionLightComponent& l) {
			RendPrim::DrawSphere(trans.WorldPosition(), pickRadius, 8, pickColor);
			RendPrim::DrawCircle(trans.WorldPosition(), trans.WorldForward(), 1.f, 8, Color{ l.Color, 1.f });

			Vec3 from = trans.WorldPosition();
			RendPrim::DrawLine(from, from + trans.WorldForward() * 0.4f, Color{ l.Color, 1.f });
			
			from = trans.WorldPosition() - trans.WorldRight() * 0.2f;
			RendPrim::DrawLine(from, from + trans.WorldForward() * 0.4f, Color{ l.Color, 1.f });

			from = trans.WorldPosition() + trans.WorldRight() * 0.2f;
			RendPrim::DrawLine(from, from + trans.WorldForward() * 0.4f, Color{ l.Color, 1.f });
			});

		m_Registry.view<TransformComponent, PointLightComponent>().each([&](TransformComponent& trans, PointLightComponent& l) {
			RendPrim::DrawSphere(trans.WorldPosition(), pickRadius, 8, pickColor);
			RendPrim::DrawSphere(trans.WorldPosition(), l.Radius, 8, Color{ l.Color, 1.f });
			});

		m_Registry.view<TransformComponent, SpotLightComponent>().each([&](TransformComponent& trans, SpotLightComponent& l) {
			RendPrim::DrawSphere(trans.WorldPosition(), pickRadius, 8, pickColor);
			RendPrim::DrawLine(trans.WorldPosition(), trans.WorldPosition() + trans.WorldForward() * 0.4f, Color{ 0, 0, 1, 1 });
			RendPrim::DrawCone(trans.WorldPosition(), trans.WorldForward(), l.CutOff, l.Radius, 8, Color{ l.Color, 1.f });
			});
	}

	void Scene::OnRenderScene(const Mat4& viewProj, const Vec3& camPos)
	{
		SceneRenderer::Environment environment;

		{
			m_Registry.view<TransformComponent, DirectionLightComponent>().each([&environment](TransformComponent &trans, DirectionLightComponent &l) {
				if (l.Enable) {
					environment.lights.push_back({});
					environment.lights.back().InitAsDirectLight(trans.WorldForward()
						, trans.WorldUp()
						, l.Color * l.Multiplier);
				}
				});

			m_Registry.view<TransformComponent, PointLightComponent>().each([&environment](TransformComponent &trans, PointLightComponent &l) {
				if (l.Enable) {
					environment.lights.push_back({});
					environment.lights.back().InitAsPointLight(trans.WorldPosition()
						, l.Color * l.Multiplier
						, l.Radius);
				}
				});

			m_Registry.view<TransformComponent, SpotLightComponent>().each([&environment](TransformComponent &trans, SpotLightComponent &l) {
				if (l.Enable) {
					environment.lights.push_back({});
					environment.lights.back().InitAsSpotLight(trans.WorldPosition()
						, trans.WorldForward()
						, l.Color * l.Multiplier
						, l.Radius
						, glm::cos(l.CutOff / 2.f));
				}
				});
		}

		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		SceneRenderer::Get().BeginScene(this, { viewProj, camPos }, environment);
		for (auto entity : group) {
			auto&[meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>(entity);
			if (meshComponent.Mesh) {
				SceneRenderer::Get().SubmitMesh(meshComponent, transformComponent.GetWorldTransform());
			}
		}
		SceneRenderer::Get().EndScene();
	}

	void Scene::OnRenderRuntime()
	{
		Entity cameraEntity = GetMainCameraEntity();
		if (!cameraEntity)
			return;

		const auto& camera = cameraEntity.GetComponent<CameraComponent>();
		HZ_CORE_ASSERT(camera.Primary);
		const auto& trans = cameraEntity.GetComponent<TransformComponent>();
		Mat4 view = glm::translate(glm::mat4(1.0f), trans.WorldPosition()) * glm::toMat4(trans.WorldRotation());
		view = glm::inverse(view);
		OnRenderScene(camera.Camera.GetProjectionMatrix() * view, trans.WorldPosition());
	}

	void Scene::OnRenderEditor(const EditorCamera& editorCamera)
	{
		OnRenderEntityInfo();
		OnRenderScene(editorCamera.GetViewProjection(), editorCamera.GetPosition());
	}

	void Scene::OnEvent(Event& e)
	{
	}

	void Scene::OnRuntimeStart()
	{
		{
			auto view = m_Registry.view<ScriptComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, this };
				s_ScriptEngine->InstantiateEntity(e);
			}
		}

		m_IsPlaying = true;
	}

	void Scene::OnRuntimeStop()
	{
		m_IsPlaying = false;
	}

	void Scene::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	Entity Scene::GetMainCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& comp = view.get<CameraComponent>(entity);
			if (comp.Primary)
				return { entity, this };
		}
		return {};
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithID(UUIDGet(), name);
	}

	Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name, bool runtimeMap)
	{
		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IDComponent>();
		idComponent.ID = uuid;

		AddTransformComponent(entity);
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		HZ_CORE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end());
		m_EntityIDMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		entity.GetComponent<TransformComponent>().DettachFromParent();
		entity.GetComponent<TransformComponent>().DettachChilds();

		if (entity.HasComponent<ScriptComponent>())
			entity.RemoveComponent<ScriptComponent>();

		auto& idComponent = entity.GetComponent<IDComponent>();
		HZ_CORE_ASSERT(m_EntityIDMap.find(idComponent.ID) != m_EntityIDMap.end());
		m_EntityIDMap.erase(idComponent.ID);
		UUIDFree(idComponent.ID);

		m_Registry.destroy(entity.m_EntityHandle);
	}

	template<typename T>
	static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto components = srcRegistry.view<T>();
		for (auto srcEntity : components)
		{
			entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

			auto& srcComponent = srcRegistry.get<T>(srcEntity);
			auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
		}
	}

	template<typename T>
	static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
	{
		if (registry.has<T>(src))
		{
			auto& srcComponent = registry.get<T>(src);
			registry.emplace_or_replace<T>(dst, srcComponent);
		}
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity;
		if (entity.HasComponent<TagComponent>())
			newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
		else
			newEntity = CreateEntity();

		CopyComponentIfExists<TransformComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<MeshComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<ScriptComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<CameraComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<DirectionLightComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<PointLightComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<SpotLightComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);

		// duplicate childs
		const auto* trans = &entity.GetComponent<TransformComponent>();
		auto* newTrans = &newEntity.GetComponent<TransformComponent>();
		newTrans->ownUUID = newEntity.GetUUID();
		newTrans->ParentUUID = UUID_INVALID;
		HZ_CORE_ASSERT(trans->ChildUUIDs.size() == newTrans->ChildUUIDs.size());
		for (int i = 0; i < trans->ChildUUIDs.size(); ++i) {
			Entity newChildEntity = DuplicateEntity(GetEntityMap().at(trans->ChildUUIDs[i]));

			// suka. create entity can trigger reallocate components buffers, so we must use ref carefully
			trans = &entity.GetComponent<TransformComponent>();
			newTrans = &newEntity.GetComponent<TransformComponent>();

			newTrans->ChildUUIDs[i] = newChildEntity.GetUUID();
			newChildEntity.GetComponent<TransformComponent>().ParentUUID = newEntity.GetUUID();
		}
		return newEntity;
	}

	Entity Scene::FindEntityByTag(const std::string& tag)
	{
		// TODO: If this becomes used often, consider indexing by tag
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const auto& canditate = view.get<TagComponent>(entity).Tag;
			if (canditate == tag)
				return Entity(entity, this);
		}

		return Entity{};
	}

	// Copy to runtime
	void Scene::CopyTo(Ref<Scene>& target)
	{
		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IDComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IDComponent>(entity).ID;
			Entity e = target->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<ScriptComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<DirectionLightComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<PointLightComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SpotLightComponent>(target->m_Registry, m_Registry, enttMap);
	}

	Ref<Scene> Scene::GetScene(UUID uuid)
	{
		if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
			return s_ActiveScenes.at(uuid);

		return {};
	}

	void Scene::AddTransformComponent(Entity entity)
	{
		auto& trans = entity.AddComponent<TransformComponent>();
		trans.pScene = this;
		trans.ownUUID = entity.GetUUID();
	}

	void Scene::DestroyAllEntities()
	{
		auto& entsMap = GetEntityMap();

		std::vector<Entity> entities;
		entities.reserve(entsMap.size());

		for (auto& [uuid, entity] : entsMap) {
			entities.push_back(entity);
		}

		for (Entity entity : entities) {
			DestroyEntity(entity);
		}
	}

}
