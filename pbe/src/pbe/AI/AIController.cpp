#include "pch.h"
#include "AIController.h"
#include "pbe/Scene/Entity.h"

namespace pbe
{
	namespace AI
	{
		Controller::Controller(Entity e, const Ref<BehaviorTree>& bt)
					: owner(std::make_unique<Entity>(e)), behaviorTree(bt)
		{
		}

		void Controller::Update()
		{
			if (behaviorTree) {
				behaviorTree->update(this);
			}
			// stearing bahavior
		}

		void Controller::SetBehaviorTree(const Ref<BehaviorTree>& bt)
		{
			behaviorTree = bt;
		}

		Ref<BehaviorTree>& Controller::GetBehaviorTree()
		{
			return behaviorTree;
		}

		void Controller::SetOwner(Entity _owner)
		{
			owner = std::make_unique<Entity>(_owner);
		}

		Entity Controller::GetOwner() const
		{
			return *owner;
		}
	}
}
