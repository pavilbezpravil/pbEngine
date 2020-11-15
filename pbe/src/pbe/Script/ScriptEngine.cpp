#include "pch.h"
#include "ScriptEngine.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>

#include <Windows.h>
#include <winioctl.h>

#include "ScriptEngineRegistry.h"

#include "pbe/Scene/Scene.h"

#include "imgui.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "ScriptWrappers.h"


namespace pbe {

	static Ref<Scene> s_SceneContext;

	static EntityInstanceMap s_EntityInstanceMap;

	using LoadedScriptMap = std::unordered_set<std::string>;
	static LoadedScriptMap s_LoadedScriptMap;

	void my_panic(sol::optional<std::string> maybe_msg) {
		HZ_CORE_WARN("Lua is in a panic state and will now abort() the application");
		if (maybe_msg) {
			const std::string& msg = maybe_msg.value();
			HZ_CORE_WARN("error message: {}", msg);
		}
	}

	int pbe_lua_exception(lua_State* L, sol::optional<const std::exception&> ex, std::string_view description) {
		// L is the lua state, which you can wrap in a state_view if necessary
		// maybe_exception will contain exception, if it exists
		// description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
		if (ex) {
			HZ_CORE_WARN("exception.what(): {}", ex->what());
		} else {
			HZ_CORE_WARN("description: {}", description);
		}

		// you must push 1 element onto the stack to be 
		// transported through as the error object in Lua
		// note that Lua -- and 99.5% of all Lua users and libraries -- expects a string
		// so we push a single string (in our case, the description of the error)
		return sol::stack::push(L, description);
	}

	sol::state g_luaState;

	void ScriptEngine::ReloadAssembly(const std::string& path)
	{
		if (s_EntityInstanceMap.size())
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			HZ_CORE_ASSERT(scene, "No active scene!");
			if (s_EntityInstanceMap.find(scene->GetUUID()) != s_EntityInstanceMap.end())
			{
				auto& entityMap = s_EntityInstanceMap.at(scene->GetUUID());
				for (auto&[entityID, entityInstanceData]: entityMap)
				{
					const auto& entityMap = scene->GetEntityMap();
					HZ_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
					InitScriptEntity(entityMap.at(entityID));
				}
			}
		}
	}

	static std::string ToLuaRequirePath(const std::string& modulePath)
	{
		std::string s = modulePath;
		std::replace(s.begin(), s.end(), '\\', '/');
		// remove '.lua'
		s.pop_back();
		s.pop_back();
		s.pop_back();
		s.pop_back();
		return s;
	}

	static std::string GetLuaModulePrefix(const std::string& modulePath)
	{
		std::string s = modulePath;
		std::replace(s.begin(), s.end(), '\\', '_');
		// remove '.lua'
		s.pop_back();
		s.pop_back();
		s.pop_back();
		s.pop_back();
		return s;
	}

	bool ScriptEngine::ReloadScript(const std::string& path)
	{
		// auto result = g_luaState.safe_script_file(path, [](lua_State*, sol::protected_function_result pfr) {
		// 	return pfr;
		// });
		auto modulePrefix = GetLuaModulePrefix(path);
		auto luaPath = ToLuaRequirePath(path);
		std::ostringstream stringStream;
		stringStream << "package.loaded['" << luaPath << "'] = nil\n"
					<< modulePrefix << " =  " << "require '" << luaPath << "'";

		auto script = stringStream.str();
		auto result = g_luaState.safe_script(script, [&](lua_State*, sol::protected_function_result pfr) {
			sol::error err = pfr;
			HZ_CORE_WARN("script '{}' reload with error: {}", path, err.what());
			return pfr;
		});

		// g_luaState.require_file(modulePrefix, luaPath + ".lua");

		return result.valid();
	}

	void ScriptEngine::ReloadAllScripts()
	{
		for (auto& scene_map : s_EntityInstanceMap) {
			for (auto& item : scene_map.second) {
				item.second.successLoaded = ScriptEngine::ReloadScript(item.second.ModulePath);
			}
		}
	}

	void ScriptEngine::Init(const std::string& assemblyPath)
	{
		HZ_CORE_INFO("ScriptEngine Init");

		g_luaState.set_panic(sol::c_call<decltype(&my_panic), &my_panic>);
		g_luaState.set_exception_handler(&pbe_lua_exception);

		g_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math);

		Script::RegisterGameFunction();
		Script::RegisterMathFunction();
		Script::RegisterComponent();
		Script::RegisterEntity();
	}

	void ScriptEngine::Shutdown()
	{
		// shutdown mono
		s_SceneContext = nullptr;
		s_EntityInstanceMap.clear();
	}

	void ScriptEngine::OnSceneDestruct(UUID sceneID)
	{
		if (s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end())
		{
			s_EntityInstanceMap.at(sceneID).clear();
			s_EntityInstanceMap.erase(sceneID);
		}
	}

	void ScriptEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		s_SceneContext = scene;
	}

	const Ref<Scene>& ScriptEngine::GetCurrentSceneContext()
	{
		return s_SceneContext;
	}

	void ScriptEngine::CopyEntityScriptData(UUID dst, UUID src)
	{
		HZ_CORE_ASSERT(s_EntityInstanceMap.find(dst) != s_EntityInstanceMap.end());
		HZ_CORE_ASSERT(s_EntityInstanceMap.find(src) != s_EntityInstanceMap.end());

		auto& dstEntityMap = s_EntityInstanceMap.at(dst);
		auto& srcEntityMap = s_EntityInstanceMap.at(src);

	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		OnCreateEntity(entity.m_Scene->GetUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void ScriptEngine::OnCreateEntity(UUID sceneID, UUID entityID)
	{
		auto& instData = GetEntityInstanceData(sceneID, entityID);
		// todo: call OnCreate
	}

	void ScriptEngine::OnUpdateEntity(UUID sceneID, UUID entityID, Timestep ts)
	{
		HZ_CORE_ASSERT(s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end());
		auto& entityMap = s_EntityInstanceMap.at(sceneID);
		HZ_CORE_ASSERT(entityMap.find(entityID) != entityMap.end());
		auto& scriptInstance = entityMap.at(entityID);

		if (!scriptInstance.successLoaded)
			return;

		auto modulePrefix = GetLuaModulePrefix(scriptInstance.ModulePath);

		auto e = s_SceneContext->GetEntityMap().at(entityID);
		auto result = g_luaState[modulePrefix]["onUpdate"](e, ts.GetSeconds());
		if (!result.valid()) {
			sol::error err = result;
			HZ_CORE_TRACE("script error: {}", err.what());
		}

		// SafeScript("OnUpdate()");
	}

	void ScriptEngine::OnScriptComponentDestroyed(UUID sceneID, UUID entityID)
	{
		HZ_CORE_ASSERT(s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end());
		auto& entityMap = s_EntityInstanceMap.at(sceneID);
		HZ_CORE_ASSERT(entityMap.find(entityID) != entityMap.end());
		entityMap.erase(entityID);
	}

	bool ScriptEngine::ScriptExists(const std::string& moduleName)
	{
		std::filesystem::path p{moduleName};
		return exists(p);
	}

	static FieldType GetPBEFieldType()
	{

		return FieldType::None;
	}

	const char* FieldTypeToString(FieldType type)
	{
		switch (type)
		{
			case FieldType::Float:       return "Float";
			case FieldType::Int:         return "Int";
			case FieldType::UnsignedInt: return "UnsignedInt";
			case FieldType::String:      return "String";
			case FieldType::Vec2:        return "Vec2";
			case FieldType::Vec3:        return "Vec3";
			case FieldType::Vec4:        return "Vec4";
		}
		return "Unknown";
	}

	void ScriptEngine::InitScriptEntity(Entity entity)
	{
		Scene* scene = entity.m_Scene;
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& scriptPath = entity.GetComponent<ScriptComponent>().ScriptPath;
		if (scriptPath.empty())
			return;

		if (!ScriptExists(scriptPath))
		{
			HZ_CORE_ERROR("Entity references non-existent script '{0}'", scriptPath);
			return;
		}

		EntityInstanceData instData;
		instData.ModulePath = scriptPath;

		if (s_LoadedScriptMap.find(scriptPath) == s_LoadedScriptMap.end()) {
			instData.successLoaded = ScriptEngine::ReloadScript(scriptPath);
			s_LoadedScriptMap.insert(scriptPath);
		}

		s_EntityInstanceMap[scene->GetUUID()][entity.GetUUID()] = instData;
	}

	void ScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& moduleName)
	{
		EntityInstanceData& entityInstanceData = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID());
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
			moduleFieldMap.erase(moduleName);
	}

	void ScriptEngine::InstantiateEntityClass(Entity entity)
	{
		Scene* scene = entity.m_Scene;
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ScriptPath;


		// Call OnCreate function (if exists)
		OnCreateEntity(entity);
	}

	bool ScriptEngine::HasEntityInstanceData(UUID sceneID, UUID entityID)
	{
		if (s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end())
		{
			auto& entityIDMap = s_EntityInstanceMap.at(sceneID);
			return entityIDMap.find(entityID) != entityIDMap.end();
		}
		return false;
	}

	EntityInstanceData& ScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
	{
		HZ_CORE_ASSERT(s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end(), "Invalid scene ID!");
		auto& entityIDMap = s_EntityInstanceMap.at(sceneID);
		HZ_CORE_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "Invalid entity ID!");
		return entityIDMap.at(entityID);
	}

	EntityInstanceMap& ScriptEngine::GetEntityInstanceMap()
	{
		return s_EntityInstanceMap;
	}

	static uint32_t GetFieldSize(FieldType type)
	{
		switch (type)
		{
			case FieldType::Float:       return 4;
			case FieldType::Int:         return 4;
			case FieldType::UnsignedInt: return 4;
			// case FieldType::String:   return 8; // TODO
			case FieldType::Vec2:        return 4 * 2;
			case FieldType::Vec3:        return 4 * 3;
			case FieldType::Vec4:        return 4 * 4;
		}
		HZ_CORE_ASSERT(false, "Unknown field type!");
		return 0;
	}

	void PublicField::CopyStoredValueToRuntime()
	{

	}

	bool PublicField::IsRuntimeAvailable() const
	{
		return true;
	}


	// Debug
	void ScriptEngine::OnImGuiRender()
	{
		static bool showDemoWindow = false;
		if (showDemoWindow) {
			ImGui::ShowDemoWindow(&showDemoWindow);
		}

		ImGui::Begin("Script Engine Debug");

		if (ImGui::Button("Reload scripts"))
			ScriptEngine::ReloadAllScripts();
		
		// for (auto& [sceneID, entityMap] : s_EntityInstanceMap)
		// {
		// 	bool opened = ImGui::TreeNode((void*)(uint64_t)sceneID, "Scene (%llx)", sceneID);
		// 	if (opened)
		// 	{
		// 		Ref<Scene> scene = Scene::GetScene(sceneID);
		// 		for (auto& [entityID, entityInstanceData] : entityMap)
		// 		{
		// 			Entity entity = scene->GetScene(sceneID)->GetEntityMap().at(entityID);
		// 			std::string entityName = "Unnamed Entity";
		// 			if (entity.HasComponent<TagComponent>())
		// 				entityName = entity.GetComponent<TagComponent>().Tag;
		// 			opened = ImGui::TreeNode((void*)(uint64_t)entityID, "%s (%llx)", entityName.c_str(), entityID);
		// 			if (opened)
		// 			{
		// 				for (auto& [moduleName, fieldMap] : entityInstanceData.ModuleFieldMap)
		// 				{
		// 					opened = ImGui::TreeNode(moduleName.c_str());
		// 					if (opened)
		// 					{
		// 						for (auto& [fieldName, field] : fieldMap)
		// 						{
		//
		// 							opened = ImGui::TreeNodeEx((void*)&field, ImGuiTreeNodeFlags_Leaf , fieldName.c_str());
		// 							if (opened)
		// 							{
		//
		// 								ImGui::TreePop();
		// 							}
		// 						}
		// 						ImGui::TreePop();
		// 					}
		// 				}
		// 				ImGui::TreePop();
		// 			}
		// 		}
		// 		ImGui::TreePop();
		// 	}
		// }
		ImGui::End();
	}

	bool ScriptEngine::SafeScript(const char* script)
	{
		auto result = g_luaState.safe_script(script, &sol::script_pass_on_error);
		if (!result.valid()) {
			sol::error err = result;
			HZ_CORE_TRACE("script error: {}", err.what());
		}
		return result.valid();
	}
}
