#pragma once

#include "Common.h"

namespace pbe {
	
	struct Transform
	{
		Transform() = default;
		Transform(const Vec3& position, const Quat& rotation = Quat_Identity, const Vec3& scale = Vec3_One);
		explicit Transform(const Quat& rotation);
		explicit Transform(const Mat4& m);
		
		static Transform FromScale(const Vec3& scale);

		Vec3 Position = Vec3_Zero;
		Quat Rotation = Quat_Identity;
		Vec3 Scale = Vec3_One;

		Mat4 GetMat4() const;
		void SetMat4(const Mat4& m);		
	};

}
