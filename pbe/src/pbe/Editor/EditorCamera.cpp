#include "pch.h"
#include "EditorCamera.h"

#include "pbe/Core/Input.h"

#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "pbe/Core/Application.h"

#define M_PI 3.14159f

namespace pbe {

	EditorCamera::EditorCamera(const glm::mat4& projectionMatrix)
		: Camera(projectionMatrix)
	{
		m_Rotation = glm::vec3(90.0f, 0.0f, 0.0f);
		m_FocalPoint = glm::vec3(0.0f);

		glm::vec3 position = { -5, 5, 5};
		m_Distance = glm::distance(position, m_FocalPoint);

		m_Yaw = 3.0f * (float)M_PI / 4.0f;
		m_Pitch = M_PI / 4.0f;

		UpdateCameraView();
	}

	void EditorCamera::UpdateCameraView()
	{
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_Rotation = glm::eulerAngles(orientation) * (180.0f / (float)M_PI);
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	void EditorCamera::Focus()
	{
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		float mouseSensitivity = 0.003f;
		
		if (Input::IsKeyPressed(KeyCode::LeftAlt)) {
			const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = (mouse - m_InitialMousePosition) * mouseSensitivity;
			m_InitialMousePosition = mouse;

			if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_MIDDLE)) {
				MousePan(delta);
			} else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT)) {
				MouseRotate(delta);
			} else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT)) {
				MouseZoom(delta.y);
			}
		} else {
			if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT)) {
				Vec3 dir = Vec3_Zero;
				if (Input::IsKeyPressed(KeyCode::W)) {
					dir += GetForwardDirection();
				}
				if (Input::IsKeyPressed(KeyCode::S)) {
					dir -= GetForwardDirection();
				}
				if (Input::IsKeyPressed(KeyCode::A)) {
					dir -= GetRightDirection();
				}
				if (Input::IsKeyPressed(KeyCode::D)) {
					dir += GetRightDirection();
				}
				if (Input::IsKeyPressed(KeyCode::Q)) {
					dir -= GetUpDirection();
				}
				if (Input::IsKeyPressed(KeyCode::E)) {
					dir += GetUpDirection();
				}

				if (glm::length2(dir) > 0.1f) {
					m_FocalPoint += glm::normalize(dir) * ts.GetSeconds() * moveSpeed;
				}

				Vec2 delta = Input::GetMouseDelta();
				if (delta != Vec2_Zero) {
					delta *= mouseSensitivity;
					
					// Vec3 pos = GetPosition();
					// Quat newOrient = Quat(Vec3(-delta.y, -delta.x, 0.f)) * GetOrientation();
					// Vec3 euler = glm::eulerAngles(newOrient);
					// m_Yaw = euler.x;
					// m_Pitch = -euler.y;
					//
					// m_FocalPoint = pos + GetForwardDirection() * m_Distance;
					MouseRotate(delta);
				}
			}
		}

		UpdateCameraView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher d(e);
		d.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(EditorCamera::OnMouseScroll));

		d.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) {
			if (e.GetMouseButton() == HZ_MOUSE_BUTTON_RIGHT) {
				Application::Get().GetWindow().SetMouseMode(MouseMode::Disabled);
				flyMode = true;
				return true;
			}
			return false;
		});
		d.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) {
			if (e.GetMouseButton() == HZ_MOUSE_BUTTON_RIGHT) {
				Application::Get().GetWindow().SetMouseMode(MouseMode::Normal);
				flyMode = false;
				return true;
			}
			return false;
		});
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		if (flyMode) {
			moveSpeed *= std::pow(1.1f, e.GetYOffset());
			moveSpeed = std::clamp(moveSpeed, minMoveSpeed, maxMoveSpeed);
		} else {
			float delta = e.GetYOffset() * 0.1f;
			MouseZoom(delta);
			UpdateCameraView();
		}
		return true;
		// return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f) {
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::GetUpDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition()
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}
}
