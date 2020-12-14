#include "pch.h"
#include "Transform.h"

#include <glm/gtx/quaternion.hpp>


namespace pbe {
	
	Transform::Transform(const Vec3& position, const Quat& rotation, const Vec3& scale)
				: Position(position), Rotation(rotation), Scale(scale)
	{
	}

	Transform::Transform(const Quat& rotation)
		: Rotation(rotation)
	{
	}

	Transform::Transform(const Mat4& m)
	{
		auto [position, rotation, scale] = GetTransformDecomposition(m);

		Position = position;
		Rotation = rotation;
		Scale = scale;
	}

	Transform Transform::FromScale(const Vec3& scale)
	{
		return Transform(Vec3_Zero, Quat_Identity, scale);
	}

	Mat4 Transform::GetMat4() const
	{
		Mat4 rotation = glm::toMat4(Rotation);

		return glm::translate(glm::mat4(1.0), Position)
			* rotation
			* glm::scale(glm::mat4(1.0), Scale);
	}

	void Transform::SetMat4(const Mat4& m)
	{
		*this = Transform(m);
	}

}
