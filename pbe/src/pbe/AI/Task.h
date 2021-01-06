#pragma once
#include "Blackboard.h"
#include "Node.h"


namespace pbe
{
	namespace AI
	{
		class TaskRegistry
		{
		public:
			using TCreateMethod = std::shared_ptr<Task>(*)();

			static TaskRegistry& Instance();

			bool Register(const std::string name, TCreateMethod funcCreate);
			std::shared_ptr<Task> Create(const std::string& name);

			std::unordered_map<std::string, TCreateMethod> TaskMap();

		private:
			std::unordered_map<std::string, TCreateMethod> s_methods;

			TaskRegistry() = default;
		};

		template <typename T>
		class AutoRegisterInRegistry
		{
		protected:
			static bool s_bRegistered;
		};

		template <typename T>
		bool AutoRegisterInRegistry<T>::s_bRegistered = TaskRegistry::Instance().Register(T::GetTaskName(), T::CreateMethod);

#define TASK_CLASS(TaskName) \
		class TaskName : public Task, public AutoRegisterInRegistry<TaskName> \
		{ \
		public: \
			TaskName() : Task(STRINGIFY(TaskName)) {} \
			void __StupidCPP() { s_bRegistered; } \
			static std::shared_ptr<Task> CreateMethod() { return std::make_shared<TaskName>(); } \
			static std::string GetTaskName() { return STRINGIFY(TaskName); }


		TASK_CLASS(MoveTo)
		Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};

		TASK_CLASS(SetValue)
		Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};
	
		TASK_CLASS(NDecrementValue)
		Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};

		TASK_CLASS(PrintEnd)
		Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};

		TASK_CLASS(RayCast)
			Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};

		TASK_CLASS(OverlapSphereAll)
			Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};
	}
}
