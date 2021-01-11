#pragma once

#include "pbe/Core/Registry.h"
#include "Blackboard.h"
#include "Node.h"


namespace pbe
{
	namespace AI
	{

#define TASK_CLASS(TaskName) \
		class TaskName : public Task, public core::AutomaticRegistryRegistration<Task, TaskName> \
		{ \
		public: \
			TaskName() : Task(STRINGIFY(TaskName)) {} \
			static std::string GetTaskName() { return STRINGIFY(TaskName); } \
			REGISTRY_TYPE(TaskName)

		using TaskRegistry = core::Registry<Task>;

		TASK_CLASS(SetMoveSpeed)
			Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};
		
		TASK_CLASS(SetRandomMoveTarget)
			Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};

		TASK_CLASS(MoveTo)
		Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};

		TASK_CLASS(SetRandomWaitTime)
		Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};

		TASK_CLASS(Wait)
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

		TASK_CLASS(FindPlayer)
			Node::Status tick(Controller* aiController, Blackboard* blackboard) override;
		};
	}
}
