#include "pch.h"
#include "Task.h"

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

		Node::Status SetValue::tick(Blackboard::Ptr& blackboard)
		{
			blackboard->GetValue("Value") = 180;
			
			// BlackboardValue v;
			// v = 180;
			// blackboard->SetValue("Value", std::move(v));
			// blackboard->SetValue("Value", v);

			HZ_CORE_INFO("SetValue {}", blackboard->GetValue("Value").As<int>());

			return Node::Status::Success;
		}

		Node::Status NDecrementValue::tick(Blackboard::Ptr& blackboard)
		{
			int& value = blackboard->GetValue("Value").As<int>();
			--value;
			return value > 0 ? Node::Status::Running : Node::Status::Success;
		}

		Node::Status PrintEnd::tick(Blackboard::Ptr& blackboard)
		{
			HZ_CORE_INFO("PrindEnd");
			return Node::Status::Success;
		}
	}
}
