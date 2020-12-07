#pragma once

#include "pbe/Core/Base.h"
#include "pbe/Core/Timestep.h"

#include <string>
#include "pbe/Scene/Entity.h"

namespace sol
{
	class state;
}

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

	using ScriptModuleFieldMap = std::unordered_map<std::string, PublicField>;

	struct ScriptModuleDesc
	{
		std::string ModulePath;
		std::string ModuleCallPrefix;
		ScriptModuleFieldMap ModuleFieldMap;
		bool successLoaded = false;
		int nUsedEntity = 0;
	};

	struct EntityInstanceData
	{
		const ScriptModuleDesc* pDesc = nullptr;
		bool awaked = false;
		bool instantiated = false;
	};

	using EntityInstanceMap = std::unordered_map<UUID, EntityInstanceData>;
	
	struct SceneScriptContext
	{
		std::unique_ptr<sol::state> luaState;
		EntityInstanceMap instanceDataMap;

		SceneScriptContext();

		bool HasInstData(Entity e);
		EntityInstanceData& GetInstData(Entity e);
		void AddInstData(Entity e, EntityInstanceData&& instData);
		void RemoveInstData(Entity e);
	};

	using ContextMap = std::unordered_map<UUID, SceneScriptContext>;

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		SceneScriptContext& GetSceneContext(Entity entity);

		void InitScene(Scene* scene);
		void ShutdownScene(Scene* scene);

		void OnAwakeEntity(Entity entity);
		void OnCreateEntity(Entity entity);
		void OnUpdateEntity(Entity entity, Timestep ts);
		void OnDestroyEntity(Entity entity);

		void OnScriptComponentDestroyed(Entity entity);

		bool PathExist(const std::string& modulePath); // todo: move to fs
		
		bool IsModuleKnown(const std::string& modulePath);
		bool IsScriptModuleSuccessLoaded(const std::string& modulePath);

		bool LoadModule(const std::string& modulePath);
		void UnloadScriptModule(const std::string& modulePath);

		void InitScriptEntity(Entity entity);
		void InstantiateEntity(Entity entity);
		void ShutdownScriptEntity(Entity entity);

		bool HasEntityInstData(Entity entity);
		EntityInstanceData& GetEntityInstData(Entity e);
		void RemoveEntityInstData(Entity e);

		// Debug
		void OnImGuiRender();

	private:
		ScriptEngine();

		void CreateContext(Scene* scene);
		void DestroyContext(Scene* scene);

		void UploadKnownModules(const sol::state& luaState);
		bool LoadModuleInternal(const sol::state& luaState, const std::string& moduleCallPrefix, const std::string& modulePath);

		using ScriptModuleDescMap = std::unordered_map<std::string, ScriptModuleDesc>;
		ScriptModuleDescMap scriptModuleDescMap;

		ContextMap contexts;

		std::unique_ptr<sol::state> testLuaState;
	};

	extern ScriptEngine* s_ScriptEngine;
}
