#include "pch.h"
#include "RendObj.h"

namespace pbe {

	void RendLight::InitAsDirectLight(const Vec3& direction, const Vec3& up, const Vec3& radiance)
	{
		type = RendLight::Direction;
		positionOrDirection = direction;
		this->up = up;
		this->radiance = radiance;
	}

	void RendLight::InitAsPointLight(const Vec3& position, const Vec3& radiance, float radius)
	{
		type = RendLight::Point;
		positionOrDirection = position;
		this->radiance = radiance;
		this->radius = radius;
	}

	void RendLight::InitAsSpotLight(const Vec3& position, const Vec3& direction, const Vec3& radiance, float radius,
		float cutOff)
	{
		type = RendLight::Spot;
		positionOrDirection = position;
		this->radiance = radiance;
		this->radius = radius;
		this->cutOff = cutOff;
		up = direction;
	}

}
