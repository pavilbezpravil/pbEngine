#include "pch.h"
#include "pbe/Core/Input.h"
#include "WindowsWindow.h"

#include "pbe/Core/Application.h"

#include <GLFW/glfw3.h>

namespace pbe {
	Input* s_Input = NULL;
	
	void Input::Init()
	{
		HZ_CORE_ASSERT(!s_Input);
		s_Input = new Input();
	}

	void Input::Shutdown()
	{
		HZ_CORE_ASSERT(s_Input);
		delete s_Input;
		s_Input = NULL;
	}

	void Input::Update()
	{
		s_Input->mousePrevPos = s_Input->mouseCurPos;
		auto [x, y] = s_Input->GetMousePosition();
		s_Input->mouseCurPos = {x, y};

		if (s_Input->mousePrevPos.x == NAN) {
			s_Input->mousePrevPos = s_Input->mouseCurPos;
		}
	}

	bool Input::IsKeyPressed(KeyCode keycode)
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());
		auto state = glfwGetKey(static_cast<GLFWwindow*>(window.GetNativeWindow()), static_cast<int32_t>(keycode));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());

		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(window.GetNativeWindow()), button);
		return state == GLFW_PRESS;
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return (float)x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return (float)y;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());

		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(window.GetNativeWindow()), &x, &y);
		return { (float)x, (float)y };

	}

	std::pair<float, float> Input::GetMouseDelta()
	{
		Vec2 delta = s_Input->mouseCurPos - s_Input->mousePrevPos;
		return {delta.x, delta.y};
	}
}
