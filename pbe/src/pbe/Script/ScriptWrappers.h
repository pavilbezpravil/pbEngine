#pragma once

#include "pbe/Script/ScriptEngine.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace pbe { namespace Script {

	// use as-is,
	// add as a member of your class,
	// or derive from it and bind it appropriately
	struct dynamic_object {
		std::unordered_map<std::string, sol::object> entries;

		void dynamic_set(std::string key, sol::stack_object value) {
			auto it = entries.find(key);
			if (it == entries.cend()) {
				entries.insert(it, { std::move(key), std::move(value) });
			} else {
				std::pair<const std::string, sol::object>& kvp = *it;
				sol::object& entry = kvp.second;
				entry = sol::object(std::move(value));
			}
		}

		sol::object dynamic_get(std::string key) {
			auto it = entries.find(key);
			if (it == entries.cend()) {
				return sol::lua_nil;
			}
			return it->second;
		}
	};

	class LuaEntity : public Entity, public dynamic_object
	{
	public:
		LuaEntity(Entity e) : Entity(e) {}
	};

	void SetScriptWrapperContext(Scene* scene);

	void LoadSystemScripts(sol::state& g_luaState);
	void RegisterGameFunction(sol::state& g_luaState);
	void RegisterMathFunction(sol::state& g_luaState);
	void RegisterColor(sol::state& luaState);
	void RegisterComponent(sol::state& g_luaState);
	void RegisterEntity(sol::state& g_luaState);
	void RegisterInput(sol::state& g_luaState);
	void RegisterRendPrim(sol::state& luaState);

} }
