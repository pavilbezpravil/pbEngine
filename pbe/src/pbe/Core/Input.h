#pragma once

#include "KeyCodes.h"
#include "Math/Common.h"

namespace pbe {

	class Input
	{
	public:
		static void Init();
		static void Shutdown();

		static void Update();

		static bool IsKeyPressed(KeyCode keycode);

		static bool IsMouseButtonPressed(int button);
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();
		static std::pair<float, float> GetMouseDelta();

	private:
		Vec2 mousePrevPos = { NAN, NAN };
		Vec2 mouseCurPos = { NAN , NAN };
	};

}
