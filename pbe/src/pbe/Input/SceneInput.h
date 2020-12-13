#pragma once

#include "pbe/Core/Math/Common.h"
#include "pbe/Core/KeyCodes.h"
#include "pbe/Core/Events/Event.h"

namespace pbe {
	
	class SceneInput : public RefCounted
	{
	public:
		void OnEvent(Event& e);

		void OnLoseFocus();
		void OnEnterFocus();

		void OnNextFrame();

		bool IsKeyPressed(KeyCode keycode) const;
		bool IsMouseButtonPressed(int button) const;
		Vec2 GetMousePos() const;
		Vec2 GetMouseDelta() const;

	private:
		Vec2 mousePrevPos = Vec2_Zero;
		Vec2 mouseCurPos = Vec2_Zero;

		std::array<bool, (int)KeyCode::Max> keyboradKeyPressed;
		std::array<bool, 3> mouseKeyPressed;

		bool mousePosKnown = false;
	};

}
