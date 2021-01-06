#include "pch.h"
#include "Task.h"
#include "AIController.h"
#include "pbe/Core/Math/Random.h"
#include "pbe/Physics/Utils.h"
#include "pbe/Renderer/RendPrim.h"
#include "pbe/Scene/Entity.h"

namespace pbe
{
	namespace AI
	{
		TaskRegistry& TaskRegistry::Instance()
		{
			static TaskRegistry r;
			return r;
		}

		bool TaskRegistry::Register(const std::string name, TCreateMethod funcCreate)
		{
			auto it = s_methods.find(name);
			if (it == s_methods.end()) {
				s_methods[name] = funcCreate;
				return true;
			}
			return false;
		}

		std::shared_ptr<Task> TaskRegistry::Create(const std::string& name)
		{
			if (auto it = s_methods.find(name); it != s_methods.end()) {
				return it->second();
			}
			return nullptr;
		}

		std::unordered_map<std::string, TaskRegistry::TCreateMethod> TaskRegistry::TaskMap()
		{
			return s_methods;
		}

		Node::Status SetMoveSpeed::tick(Controller* aiController, Blackboard* blackboard)
		{
			blackboard->GetValue("MoveSpeed") = s_Random.Float(1.f, 5.f);
			return Node::Status::Success;
		}

		Node::Status SetRandomMoveTarget::tick(Controller* aiController, Blackboard* blackboard)
		{
			Vec2 xz = s_Random.InSquare() * Vec2{20.f};
			blackboard->GetValue("MoveTarget") = Vec3{xz.x, 1.5f, xz.y};
			return Node::Status::Success;
		}

		Node::Status MoveTo::tick(Controller* aiController, Blackboard* blackboard)
		{
			Vec3 target = blackboard->GetValue("MoveTarget").As<Vec3>();
			float speed = blackboard->GetValue("MoveSpeed").As<float>();

			RendPrim::DrawSphere(target, 0.5f, 16, Color_Green);
			
			Entity e = aiController->GetOwner();
			auto& trans = e.GetComponent<TransformComponent>();

			if (glm::length(target - trans.WorldPosition()) < 0.2f) {
				return Node::Status::Success;
			}
			
			Vec3 direction = glm::normalize((target - trans.WorldPosition()));
			if (e.HasComponent<RigidbodyComponent>()) {
				auto& rb = e.GetComponent<RigidbodyComponent>();
				rb.SetVelocity(direction * speed);
			} else {
				return Node::Status::Failure;
			}

			return Node::Status::Running;
		}

		Node::Status SetRandomWaitTime::tick(Controller* aiController, Blackboard* blackboard)
		{
			blackboard->GetValue("WaitTime") = s_Random.Float(1.f, 5.f);
			HZ_CORE_INFO("SetWaitTime {}", blackboard->GetValue("WaitTime").As<float>());
			return Node::Status::Success;
		}

		Node::Status Wait::tick(Controller* aiController, Blackboard* blackboard)
		{
			// todo:
			float dt = 1.f / 60.f;
			
			float& waitTime = blackboard->GetValue("WaitTime").As<float>();
			waitTime -= dt;
			HZ_CORE_INFO("WaitTime {}", waitTime);
			return waitTime < 0.f ? Node::Status::Success : Node::Status::Running;
		}

		Node::Status SetValue::tick(Controller* aiController, Blackboard* blackboard)
		{
			blackboard->GetValue("Value") = 180;
			HZ_CORE_INFO("SetValue {}", blackboard->GetValue("Value").As<int>());
			return Node::Status::Success;
		}

		Node::Status NDecrementValue::tick(Controller* aiController, Blackboard* blackboard)
		{
			int& value = blackboard->GetValue("Value").As<int>();
			--value;
			return value > 0 ? Node::Status::Running : Node::Status::Success;
		}

		Node::Status PrintEnd::tick(Controller* aiController, Blackboard* blackboard)
		{
			HZ_CORE_INFO("PrindEnd");
			return Node::Status::Success;
		}

		Node::Status RayCast::tick(Controller* aiController, Blackboard* blackboard)
		{
			Entity e = aiController->GetOwner();
			auto& trans = e.GetComponent<TransformComponent>();

			physics::RaycastHit hit;
			bool status = SceneRayCast(e.GetScene(), trans.WorldPosition() + trans.WorldForward() * 1.5f, trans.WorldForward(), 100.f, hit);
			return status ? Node::Status::Success : Node::Status::Running;
		}

		Node::Status OverlapSphereAll::tick(Controller* aiController, Blackboard* blackboard)
		{
			Entity e = aiController->GetOwner();
			auto& trans = e.GetComponent<TransformComponent>();

			auto overlaped = physics::SceneOverlapSphereAll(e.GetScene(), trans.WorldPosition() + trans.WorldForward() * 2.f, 5.f);
			return !overlaped.empty() ? Node::Status::Success : Node::Status::Running;
		}
	}
}
