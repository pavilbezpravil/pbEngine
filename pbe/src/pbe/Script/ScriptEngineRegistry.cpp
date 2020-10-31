#include "pch.h"
#include "ScriptEngineRegistry.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pbe/Scene/Entity.h"
#include "ScriptWrappers.h"
#include <iostream>

namespace pbe {

	std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;

	extern MonoImage* s_CoreAssemblyImage;

#define Component_RegisterType(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("pbe." #Type, s_CoreAssemblyImage);\
		if (type) {\
			uint32_t id = mono_type_get_type(type);\
			s_HasComponentFuncs[type] = [](Entity& entity) { return entity.HasComponent<Type>(); };\
			s_CreateComponentFuncs[type] = [](Entity& entity) { entity.AddComponent<Type>(); };\
		} else {\
			HZ_CORE_ERROR("No C# component class found for " #Type "!");\
		}\
	}

	static void InitComponentTypes()
	{
		Component_RegisterType(TagComponent);
		Component_RegisterType(TransformComponent);
		Component_RegisterType(MeshComponent);
		Component_RegisterType(ScriptComponent);
		Component_RegisterType(CameraComponent);
		Component_RegisterType(SpriteRendererComponent);
		Component_RegisterType(RigidBody2DComponent);
		Component_RegisterType(BoxCollider2DComponent);
	}

	void ScriptEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

		mono_add_internal_call("pbe.Noise::PerlinNoise_Native", pbe::Script::pbe_Noise_PerlinNoise);

		mono_add_internal_call("pbe.Entity::GetTransform_Native", pbe::Script::pbe_Entity_GetTransform);
		mono_add_internal_call("pbe.Entity::SetTransform_Native", pbe::Script::pbe_Entity_SetTransform);
		mono_add_internal_call("pbe.Entity::CreateComponent_Native", pbe::Script::pbe_Entity_CreateComponent);
		mono_add_internal_call("pbe.Entity::HasComponent_Native", pbe::Script::pbe_Entity_HasComponent);
		mono_add_internal_call("pbe.Entity::FindEntityByTag_Native", pbe::Script::pbe_Entity_FindEntityByTag);

		mono_add_internal_call("pbe.MeshComponent::GetMesh_Native", pbe::Script::pbe_MeshComponent_GetMesh);
		mono_add_internal_call("pbe.MeshComponent::SetMesh_Native", pbe::Script::pbe_MeshComponent_SetMesh);

		mono_add_internal_call("pbe.RigidBody2DComponent::ApplyLinearImpulse_Native", pbe::Script::pbe_RigidBody2DComponent_ApplyLinearImpulse);
		mono_add_internal_call("pbe.RigidBody2DComponent::GetLinearVelocity_Native", pbe::Script::pbe_RigidBody2DComponent_GetLinearVelocity);
		mono_add_internal_call("pbe.RigidBody2DComponent::SetLinearVelocity_Native", pbe::Script::pbe_RigidBody2DComponent_SetLinearVelocity);

		mono_add_internal_call("pbe.Input::IsKeyPressed_Native", pbe::Script::pbe_Input_IsKeyPressed);

		mono_add_internal_call("pbe.Texture2D::Constructor_Native", pbe::Script::pbe_Texture2D_Constructor);
		mono_add_internal_call("pbe.Texture2D::Destructor_Native", pbe::Script::pbe_Texture2D_Destructor);
		mono_add_internal_call("pbe.Texture2D::SetData_Native", pbe::Script::pbe_Texture2D_SetData);

		mono_add_internal_call("pbe.Mesh::Constructor_Native", pbe::Script::pbe_Mesh_Constructor);
		mono_add_internal_call("pbe.Mesh::Destructor_Native", pbe::Script::pbe_Mesh_Destructor);

		mono_add_internal_call("pbe.MeshFactory::CreatePlane_Native", pbe::Script::pbe_MeshFactory_CreatePlane);

		// static bool IsKeyPressed(KeyCode key) { return s_Instance->IsKeyPressedImpl(key); }
		// 
		// static bool IsMouseButtonPressed(MouseCode button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		// static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		// static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		// static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
	}

}
