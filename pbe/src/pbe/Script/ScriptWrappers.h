#pragma once

#include "pbe/Script/ScriptEngine.h"
#include "pbe/Core/KeyCodes.h"

#include <glm/glm.hpp>


namespace pbe { namespace Script {


	void RegisterGameFunction();
	void RegisterMathFunction();
	void RegisterComponent();
	void RegisterEntity();

} }
