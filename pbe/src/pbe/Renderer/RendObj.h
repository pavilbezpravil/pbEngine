#pragma once

#include "Mesh.h"
#include "ColorBuffer.h"
#include "pbe/Core/Math/Transform.h"
#include "pbe/Scene/SceneCamera.h"


namespace pbe {
	
	struct RendObj
	{
		AABB aabb;
		Transform transform;
		int instIndx = -1;
	};

	struct RendObjMesh : RendObj
	{
		Ref<Mesh> mesh;
	};

	struct RendLight
	{
		enum Type
		{
			Direction,
			Point,
			Spot,
		};

		Vec3 positionOrDirection = { 0.0f, 0.0f, 0.0f };
		float radius;
		Vec3 radiance = { 0.0f, 0.0f, 0.0f };
		float cutOff;
		Type type;

		Vec3 up; // in case spot light direction

		void InitAsDirectLight(const Vec3& direction, const Vec3& up, const Vec3& radiance);
		void InitAsPointLight(const Vec3& position, const Vec3& radiance, float radius);
		void InitAsSpotLight(const Vec3& position, const Vec3& direction, const Vec3& radiance, float radius,
		                     float cutOff);
	};

	struct RendCamera : SceneCamera
	{
		Vec3 position;
		Ref<ColorBuffer> target;
	};

}
