#pragma once
#include "BehaviorTree.h"
#include "pbe/Core/Ref.h"


namespace pbe
{
	class Entity;
	
	namespace AI
	{
		class Controller : public RefCounted
		{
		public:
			Controller(Entity e, const Ref<BehaviorTree>& bt = {});

			void Update();
			
			void SetBehaviorTree(const Ref<BehaviorTree>& bt);
			Ref<BehaviorTree>& GetBehaviorTree();

			void SetOwner(Entity owner);
			Entity GetOwner() const;
			
		private:
			std::unique_ptr<Entity> owner;
			Ref<BehaviorTree> behaviorTree;
		};

	}
}
