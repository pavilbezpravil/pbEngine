#pragma once

#include "pbe/Core/Base.h"
#include "pbe/Core/Timestep.h"

#include <string>

#include "pbe/Scene/Components.h"
#include "pbe/Scene/Entity.h"


namespace pbe {

	enum class FieldType
	{
		None = 0, Float, Int, UnsignedInt, String, Vec2, Vec3, Vec4
	};

	const char* FieldTypeToString(FieldType type);

	struct PublicField
	{
		std::string Name;
		FieldType Type;

		void CopyStoredValueToRuntime();
		bool IsRuntimeAvailable() const;

	private:

		friend class ScriptEngine;
	};

	using ScriptModuleFieldMap = std::unordered_map<std::string, std::unordered_map<std::string, PublicField>>;

	struct EntityInstanceData
	{
		std::string ModulePath;
		ScriptModuleFieldMap ModuleFieldMap;
		bool successLoaded = false;
	};

	using EntityInstanceMap = std::unordered_map<UUID, std::unordered_map<UUID, EntityInstanceData>>;

	class ScriptEngine
	{
	public:
		static void Init(const std::string& assemblyPath);
		static void Shutdown();

		static void OnSceneDestruct(UUID sceneID);

		static void ReloadAssembly(const std::string& path);
		static bool ReloadScript(const std::string& path);
		static void ReloadAllScripts();

		static void SetSceneContext(const Ref<Scene>& scene);
		static const Ref<Scene>& GetCurrentSceneContext();

		static void CopyEntityScriptData(UUID dst, UUID src);

		static void OnCreateEntity(Entity entity);
		static void OnCreateEntity(UUID sceneID, UUID entityID);
		static void OnUpdateEntity(UUID sceneID, UUID entityID, Timestep ts);

		static void OnScriptComponentDestroyed(UUID sceneID, UUID entityID);

		static bool ScriptExists(const std::string& moduleName);
		static void InitScriptEntity(Entity entity);
		static void ShutdownScriptEntity(Entity entity, const std::string& moduleName);
		static void InstantiateEntityClass(Entity entity);

		static EntityInstanceMap& GetEntityInstanceMap();
		static bool HasEntityInstanceData(UUID sceneID, UUID entityID);
		static EntityInstanceData& GetEntityInstanceData(UUID sceneID, UUID entityID);

		// Debug
		static void OnImGuiRender();

	private:
		static bool SafeScript(const char* script);
	};
}
