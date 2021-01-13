#include "pch.h"
#include "ScriptEngine.h"

#include <filesystem>
#include <imgui.h>
#include <Windows.h>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "pbe/Scene/Scene.h"

#include "ScriptWrappers.h"
#include "pbe/Scene/StandartComponents.h"


namespace pbe {

	ScriptEngine* s_ScriptEngine = NULL;

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

	static std::string ToLuaRequirePath(const std::string& modulePath)
	{
		std::string s = modulePath;
		std::replace(s.begin(), s.end(), '\\', '/');
		// // remove '.lua'
		// s.pop_back();
		// s.pop_back();
		// s.pop_back();
		// s.pop_back();
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

	static sol::light<uint64_t> GetEntityHandler(Entity e)
	{
		void* handler = (void*)(uint64_t)e.GetUUID();
		return sol::light<uint64_t>(handler);
	}

	template<typename... Ts>
	std::ostream& operator<<(std::ostream& os, std::tuple<Ts...> const& theTuple)
	{
		std::apply
		(
			[&os](Ts const&... tupleArgs)
			{
				os << '[';
				std::size_t n{ 0 };
				((os << tupleArgs << (++n != sizeof...(Ts) ? ", " : "")), ...);
				os << ']';
			}, theTuple
		);
		return os;
	}
	
	template<typename Func, typename... Args>
	bool LuaSafeCall(Func&& func, Args&&... args)
	{
		if (func != sol::nil) {
			auto result = func(std::forward<Args>(args)...);
			if (!result.valid()) {
				sol::error err = result;
				HZ_CORE_TRACE("script error: {}", err.what());
			}
			return result.valid();
		} else {
			std::ostringstream iss;
			iss << func.key;
			HZ_CORE_TRACE("{} is nil", iss.str());
		}
		return false;

		// try
		// {
		// 	func(std::forward<Args>(args)...);
		// }
		// catch (sol::error err) {
		// 	HZ_CORE_TRACE("script error: {}", err.what());
		// 	return false;
		// }
	}

	void MakeDefaultState(sol::state& luaState)
	{
		luaState = {};
		luaState.set_panic(sol::c_call<decltype(&my_panic), &my_panic>);
		luaState.set_exception_handler(&pbe_lua_exception);

		luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math, sol::lib::debug);

		Script::LoadSystemScripts(luaState);
		Script::RegisterGameFunction(luaState);
		Script::RegisterInput(luaState);
		Script::RegisterRendPrim(luaState);
		Script::RegisterMathFunction(luaState);
		Script::RegisterColor(luaState);
		Script::RegisterComponent(luaState);
		Script::RegisterScene(luaState);
		Script::RegisterEntity(luaState);
	}

	SceneScriptContext::SceneScriptContext() : luaState(std::make_unique<sol::state>()) {
		
	}

	bool SceneScriptContext::HasInstData(Entity e)
	{
		return instanceDataMap.count(e.GetUUID()) > 0;
	}

	void ScriptEngine::InitScene(Scene* scene)
	{
		CreateContext(scene);
	}

	void ScriptEngine::ShutdownScene(Scene* scene)
	{
		DestroyContext(scene);
	}

	void ScriptEngine::SetContext(Scene* scene)
	{
		Script::SetScriptWrapperContext(scene);
	}

	EntityInstanceData& SceneScriptContext::GetInstData(Entity e)
	{
		HZ_CORE_ASSERT(instanceDataMap.count(e.GetUUID()));
		return instanceDataMap[e.GetUUID()];
	}

	void SceneScriptContext::AddInstData(Entity e, EntityInstanceData&& instData)
	{
		HZ_CORE_ASSERT(instanceDataMap.count(e.GetUUID()) == 0);
		instanceDataMap[e.GetUUID()] = std::move(instData);
	}

	void SceneScriptContext::RemoveInstData(Entity e)
	{
		HZ_CORE_ASSERT(instanceDataMap.count(e.GetUUID()));
		instanceDataMap.erase(e.GetUUID());
	}

	void ScriptEngine::Init()
	{
		HZ_CORE_INFO("ScriptEngine Init");

		s_ScriptEngine = new ScriptEngine;
	}

	void ScriptEngine::Shutdown()
	{
		delete s_ScriptEngine;
		s_ScriptEngine = NULL;
	}

	SceneScriptContext& ScriptEngine::GetSceneContext(Entity entity)
	{
		HZ_CORE_ASSERT(contexts.find(entity.GetSceneUUID()) != contexts.end());
		return contexts[entity.GetSceneUUID()];
	}

	void ScriptEngine::OnAwakeEntity(Entity entity)
	{
		// todo:
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		EntityInstanceData& instData = GetEntityInstData(entity);
		const ScriptModuleDesc& smDesc = *instData.pDesc;
		if (!smDesc.successLoaded)
			return;

		auto& luaState = *GetSceneContext(entity).luaState;

		auto entityHandler = GetEntityHandler(entity);
		Script::LuaEntity& e = luaState["pbe_entity"]["map"][entityHandler];

		auto func = luaState[smDesc.ModuleCallPrefix]["onCreate"];
		LuaSafeCall(func, e);
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		EntityInstanceData& instData = GetEntityInstData(entity);
		const ScriptModuleDesc& smDesc = *instData.pDesc;
		if (!smDesc.successLoaded)
			return;

		auto& luaState = *GetSceneContext(entity).luaState;

		// luaState["pbe_entity"]["printEntityInfo"]();
		// luaState["pbe_entity"]["printEntityMap"]();


		auto entityHandler = GetEntityHandler(entity);
		// auto& res = luaState["pbe_entity"]["getEntity"](entityHandler);
		// auto& res = ;
		Script::LuaEntity& e = luaState["pbe_entity"]["map"][entityHandler];
		// Entity& e = luaState["pbe_entity"]["map"][entityHandler];
		
		auto func = luaState[smDesc.ModuleCallPrefix]["onUpdate"];
		LuaSafeCall(func, e, ts.GetSeconds());

		// auto func = luaState[smDesc.ModuleCallPrefix]["onUpdate"];
		// LuaSafeCall(func, entity, ts.GetSeconds());
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		EntityInstanceData& instData = GetEntityInstData(entity);
		if (!instData.instantiated)
			return;

		auto func = (*GetSceneContext(entity).luaState)[instData.pDesc->ModuleCallPrefix]["onDestroy"];
		LuaSafeCall(func, entity);
	}

	void ScriptEngine::OnBeginOverlap(Entity entity, Entity other)
	{
		if (HasEntityInstData(entity)) {
			EntityInstanceData& instData = GetEntityInstData(entity);
			if (!instData.instantiated)
				return;

			auto func = (*GetSceneContext(entity).luaState)[instData.pDesc->ModuleCallPrefix]["onBeginOverlap"];
			LuaSafeCall(func, entity, other);
		}
	}

	void ScriptEngine::OnTriggerEnter(Entity entity, Entity trigger)
	{
		if (HasEntityInstData(entity)) {
			EntityInstanceData& instData = GetEntityInstData(entity);
			if (!instData.instantiated)
				return;

			auto func = (*GetSceneContext(entity).luaState)[instData.pDesc->ModuleCallPrefix]["onTriggerEnter"];
			LuaSafeCall(func, entity, trigger);
		}
	}

	void ScriptEngine::OnTriggerExit(Entity entity, Entity trigger)
	{
		if (HasEntityInstData(entity)) {
			EntityInstanceData& instData = GetEntityInstData(entity);
			if (!instData.instantiated)
				return;

			auto func = (*GetSceneContext(entity).luaState)[instData.pDesc->ModuleCallPrefix]["onTriggerExit"];
			LuaSafeCall(func, entity, trigger);
		}
	}

	bool ScriptEngine::PathExist(const std::string& modulePath)
	{
		std::filesystem::path p{modulePath};
		return exists(p);
	}

	bool ScriptEngine::IsModuleKnown(const std::string& modulePath)
	{
		return scriptModuleDescMap.find(modulePath) != scriptModuleDescMap.end();
	}

	bool ScriptEngine::IsScriptModuleSuccessLoaded(const std::string& modulePath)
	{
		return IsModuleKnown(modulePath) && scriptModuleDescMap[modulePath].successLoaded;
	}

	bool ScriptEngine::LoadModule(const std::string& modulePath)
	{
		scriptModuleDescMap[modulePath] = {};
		
		ScriptModuleDesc& smDesc = scriptModuleDescMap[modulePath];
		smDesc.ModulePath = modulePath;
		smDesc.ModuleCallPrefix = GetLuaModulePrefix(modulePath);

		std::string modulePathProcessed = ToLuaRequirePath(modulePath);
		bool success = LoadModuleInternal(*testLuaState, smDesc.ModuleCallPrefix, modulePathProcessed);
		// todo: stupid logic
		if (success) {
			for (auto& element : contexts) {
				LoadModuleInternal(*element.second.luaState, smDesc.ModuleCallPrefix, modulePathProcessed);
			}
		} else {
			HZ_CORE_TRACE("Cant load module {}", smDesc.ModulePath);
		}

		smDesc.successLoaded = success;
		return smDesc.successLoaded;
	}

	void ScriptEngine::UnloadScriptModule(const std::string& modulePath)
	{
		HZ_CORE_ASSERT(IsModuleKnown(modulePath));
		scriptModuleDescMap.erase(modulePath);
	}

	void ScriptEngine::InitScriptEntity(Entity entity)
	{
		auto& sc = entity.GetComponent<ScriptComponent>();
		if (!IsModuleKnown(sc.ScriptPath)) {
			LoadModule(sc.ScriptPath);
		}

		HZ_CORE_ASSERT(!HasEntityInstData(entity));
		GetSceneContext(entity).AddInstData(entity, {});
		EntityInstanceData& instData = GetEntityInstData(entity);
		instData.pDesc = &scriptModuleDescMap[sc.ScriptPath];

		if (IsScriptModuleSuccessLoaded(sc.ScriptPath)) {
			instData.awaked = true;
			UploadEntityToState(entity);
			OnAwakeEntity(entity);
		}
	}

	void ScriptEngine::InstantiateEntity(Entity entity)
	{
		auto& instData = GetEntityInstData(entity);
		if (IsScriptModuleSuccessLoaded(instData.pDesc->ModulePath)) {
			instData.instantiated = true;
			OnCreateEntity(entity);
		}
	}

	void ScriptEngine::ShutdownScriptEntity(Entity entity)
	{
		HZ_CORE_ASSERT(HasEntityInstData(entity));
		auto& instData = GetEntityInstData(entity);
		if (instData.instantiated) {
			HZ_CORE_ASSERT(IsScriptModuleSuccessLoaded(instData.pDesc->ModulePath));
			OnDestroyEntity(entity);
			UnloadEntityFromState(entity);
			instData.instantiated = false;
		}
		instData.awaked = false;
		RemoveEntityInstData(entity);
	}

	bool ScriptEngine::HasEntityInstData(Entity entity)
	{
		return GetSceneContext(entity).HasInstData(entity);
	}

	EntityInstanceData& ScriptEngine::GetEntityInstData(Entity e)
	{
		return GetSceneContext(e).GetInstData(e);
	}

	void ScriptEngine::RemoveEntityInstData(Entity e)
	{
		GetSceneContext(e).RemoveInstData(e);
	}

	void ScriptEngine::OnImGuiRender()
	{
		
	}

	ScriptEngine::ScriptEngine()
	{
		testLuaState = std::make_unique<sol::state>();
		MakeDefaultState(*testLuaState);
	}

	void ScriptEngine::CreateContext(Scene* scene)
	{
		HZ_CORE_ASSERT(contexts.count(scene->GetUUID()) == 0);

		contexts[scene->GetUUID()] = {};
		auto& c = contexts[scene->GetUUID()];

		MakeDefaultState(*c.luaState);
		UploadKnownModules(*c.luaState);
	}

	void ScriptEngine::DestroyContext(Scene* scene)
	{
		HZ_CORE_ASSERT(contexts.find(scene->GetUUID()) != contexts.end());
		contexts.erase(scene->GetUUID());
	}

	void ScriptEngine::UploadKnownModules(const sol::state& luaState)
	{
		for (auto& [modulePath, moduleDesc]: scriptModuleDescMap) {
			LoadModuleInternal(luaState, moduleDesc.ModuleCallPrefix, moduleDesc.ModulePath);
		}
	}

	bool ScriptEngine::LoadModuleInternal(const sol::state& luaState, const std::string& moduleCallPrefix,
		const std::string& modulePath)
	{
		bool successLoad = LuaSafeCall((luaState)["pbe_sys"]["loadModule"], moduleCallPrefix, modulePath);
		return successLoad;
	}

	void ScriptEngine::UploadEntityToState(Entity entity)
	{
		auto& luaState = *GetSceneContext(entity).luaState;
		auto entityHandler = GetEntityHandler(entity);
		// LuaSafeCall(luaState["pbe_entity"]["addEntity"], entityHandler, entity);
		luaState["pbe_entity"]["map"][entityHandler] = Script::LuaEntity{ entity };
		// luaState["pbe_entity"]["map"][entityHandler] = entity;
	}

	void ScriptEngine::GetEntityFromState(Entity entity)
	{
		// auto& luaState = *GetSceneContext(entity).luaState;
		// void* handler = (void*)(uint64_t)entity.GetUUID();
		// auto e = luaState["pbe_entity"]["getEntity"](sol::light(handler)).get<sol::table>();
		// e["sdfsdf"];
		//
		// auto i = luaState["pbe_entity"];
		//
	}

	void ScriptEngine::UnloadEntityFromState(Entity entity)
	{
		auto& luaState = *GetSceneContext(entity).luaState;
		auto entityHandler = GetEntityHandler(entity);
		LuaSafeCall(luaState["pbe_entity"]["removeEntity"], entityHandler);
	}
}
