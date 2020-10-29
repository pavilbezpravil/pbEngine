#include "pch.h"
#include "Camera.h"

namespace pbe {

	Camera::Camera(const glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix)
	{
	}

}