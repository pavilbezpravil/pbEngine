#include "pch.h"
#include "ScriptEngineRegistry.h"


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pbe/Scene/Entity.h"
#include "ScriptWrappers.h"
#include <iostream>

namespace pbe {

#define Component_RegisterType(Type)

	static void InitComponentTypes()
	{
		Component_RegisterType(TagComponent);
		Component_RegisterType(TransformComponent);
		Component_RegisterType(MeshComponent);
		Component_RegisterType(ScriptComponent);
		Component_RegisterType(CameraComponent);
		Component_RegisterType(DirectionLightComponent);
	}

	void ScriptEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

	}

}
