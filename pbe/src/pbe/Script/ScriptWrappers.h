#pragma once

#include "pbe/Script/ScriptEngine.h"
#include "pbe/Core/KeyCodes.h"

#include <glm/glm.hpp>

extern "C" {
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}

namespace pbe { namespace Script {

	// Math
	float pbe_Noise_PerlinNoise(float x, float y);

	// Input
	bool pbe_Input_IsKeyPressed(KeyCode key);

	// Entity
	void pbe_Entity_GetTransform(uint64_t entityID, glm::mat4* outTransform);
	void pbe_Entity_SetTransform(uint64_t entityID, glm::mat4* inTransform);
	void pbe_Entity_CreateComponent(uint64_t entityID, void* type);
	bool pbe_Entity_HasComponent(uint64_t entityID, void* type);
	uint64_t pbe_Entity_FindEntityByTag(MonoString* tag);

	void* pbe_MeshComponent_GetMesh(uint64_t entityID);
	void pbe_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh);

	void pbe_RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2* impulse, glm::vec2* offset, bool wake);
	void pbe_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity);
	void pbe_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, glm::vec2* velocity);

	// Renderer
	// Texture2D
	void* pbe_Texture2D_Constructor(uint32_t width, uint32_t height);
	void pbe_Texture2D_Destructor(Ref<Texture2D>* _this);
	void pbe_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count);

	// Mesh
	Ref<Mesh>* pbe_Mesh_Constructor(MonoString* filepath);
	void pbe_Mesh_Destructor(Ref<Mesh>* _this);

	void* pbe_MeshFactory_CreatePlane(float width, float height);
} }
