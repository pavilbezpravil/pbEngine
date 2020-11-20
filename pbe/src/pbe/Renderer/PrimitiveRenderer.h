#pragma once
#include "pbe/Core/Math/Color.h"
#include "pbe/Core/Math/Common.h"


namespace pbe {

	namespace Debug
	{
		struct Point
		{
			Vec3 position;
			Color color;
		};
		
		// void OnImGui();

		void Init();
		void Term();

		void DrawLine(const Vec3& from, const Vec3& to, const Color& color);
	}

}
