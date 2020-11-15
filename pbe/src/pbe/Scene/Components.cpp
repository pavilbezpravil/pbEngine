#include "pch.h"
#include "Components.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace pbe
{
	Mat4 TransformComponent::GetTransform() const
	{
		Mat4 rotation = glm::toMat4(Rotation);
		// glm::rotate(glm::mat4(1.0), Rotation.x, { 1, 0, 0})
	  // * glm::rotate(glm::mat4(1.0), Rotation.y, { 0, 1, 0})
	  // * glm::rotate(glm::mat4(1.0), Rotation.z, { 0, 0, 1});

		return glm::translate(glm::mat4(1.0), Translation)
			* rotation
			* glm::scale(glm::mat4(1.0), Scale);
	}

	void TransformComponent::SetTransform(const Mat4& trans)
	{
		auto[translate, rotate, scale] = GetTransformDecomposition(trans);

		Translation = translate;
		Rotation = rotate;
		// Rotation = glm::eulerAngles(rotate);
		Scale = scale;
	}

}
