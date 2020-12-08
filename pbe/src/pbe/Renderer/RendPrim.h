#pragma once
#include "CommandContext.h"
#include "pbe/Core/Math/Color.h"
#include "pbe/Core/Math/Common.h"


namespace pbe {
	struct AABB;
}

namespace pbe {

	namespace RendPrim
	{
		struct RendPoint
		{
			Vec3 position;
			Color color;
		};

		void Init();
		void Term();

		void RenderPrimitives(GraphicsContext& context, const Mat4& viewProj);

		void DrawLine(const Vec3& from, const Vec3& to, const Color& color);
		void DrawAABB(const AABB& aabb, const Color& color);
		void DrawCircle(const Vec3& center, const Vec3& normal, float radius, uint nSegments, const Color& color);
		void DrawSphere(const Vec3& center, float radius, uint nSegments, const Color& color);
		void DrawCone(const Vec3& center, const Vec3& forward, float angle, float radius, uint nSegments, const Color& color);
	}

}
