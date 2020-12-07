#pragma once

#include "pbe/Script/ScriptEngine.h"
#include "pbe/Core/KeyCodes.h"

#include <glm/glm.hpp>


namespace pbe { namespace Script {

	void LoadSystemScripts(sol::state& g_luaState);
	void RegisterGameFunction(sol::state& g_luaState);
	void RegisterMathFunction(sol::state& g_luaState);
	void RegisterComponent(sol::state& g_luaState);
	void RegisterEntity(sol::state& g_luaState);
	void RegisterInput(sol::state& g_luaState);

} }
