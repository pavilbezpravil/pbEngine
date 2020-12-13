#include "pch.h"
#include "SceneInput.h"

#include "pbe/Core/Events/MouseEvent.h"
#include "pbe/Core/Events/KeyEvent.h"

namespace pbe {

	void SceneInput::OnEvent(Event& e)
	{
		EventDispatcher d(e);
		
		d.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) {
			mouseKeyPressed[e.GetMouseButton()] = true;
			return false;
			});
		d.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) {
			mouseKeyPressed[e.GetMouseButton()] = false;
			return false;
			});
		// d.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e) {
		// 	io.MouseWheel = e.GetYOffset();
		// 	io.MouseWheelH = e.GetXOffset();
		// 	return isMouseHandles;
		// 	});
		d.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) {
			mouseCurPos = { e.GetX(), e.GetY() };
			mousePosKnown = true;
			return false;
			});

		d.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e) {
			keyboradKeyPressed[(int)e.GetKeyCode()] = true;
			return false;
			});

		d.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e) {
			keyboradKeyPressed[(int)e.GetKeyCode()] = false;
			return false;
			});

		// d.Dispatch<KeyTypedEvent>([&](KeyTypedEvent& e) {
		// 	io.AddInputCharacter((uint)e.GetKeyCode());
		// 	return isKeyboardHandles;
		// 	});
	}

	void SceneInput::OnLoseFocus()
	{
		std::fill(keyboradKeyPressed.begin(), keyboradKeyPressed.end(), false);
		std::fill(mouseKeyPressed.begin(), mouseKeyPressed.end(), false);
		
		mousePosKnown = false;
	}

	void SceneInput::OnEnterFocus()
	{
	}

	void SceneInput::OnNextFrame()
	{
		mousePrevPos = mouseCurPos;
	}

	bool SceneInput::IsKeyPressed(KeyCode keycode) const
	{
		return keyboradKeyPressed[(int)keycode];
	}

	bool SceneInput::IsMouseButtonPressed(int button) const
	{
		return mouseKeyPressed[button];
	}

	Vec2 SceneInput::GetMousePos() const
	{
		return mouseCurPos;
	}

	Vec2 SceneInput::GetMouseDelta() const
	{
		return mousePosKnown ? (mouseCurPos - mousePrevPos) : Vec2_Zero;
	}

}
