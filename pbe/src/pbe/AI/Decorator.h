#pragma once
#include "pbe/Core/Registry.h"
#include "Node.h"

namespace pbe
{
	namespace AI
	{
		#define DECORATOR_CLASS(Name) \
			class Name : public Decorator, public core::AutomaticRegistryRegistration<Decorator, Name> \
			{ \
			public: \
				Name() : Decorator(STRINGIFY(Name)) {} \
				REGISTRY_TYPE(Name)

		using DecoratorRegistry = core::Registry<Decorator>;

		DECORATOR_CLASS(Root)
			Status update() { return hasChild() ? child->tick() : Status::Success; }
		};

		DECORATOR_CLASS(Succeeder)
			Status update() override
			{
				child->tick();
				return Status::Success;
			}
		};

		DECORATOR_CLASS(Failer)
			Status update() override
			{
				child->tick();
				return Status::Failure;
			}
		};

		DECORATOR_CLASS(Inverter)
			Status update() override
			{
				auto s = child->tick();
		
				if (s == Status::Success) {
					return Status::Failure;
				}
				else if (s == Status::Failure) {
					return Status::Success;
				}
		
				return s;
			}
		};

		// DECORATOR_CLASS(Repeater)
		// 	Repeater(int limit = 0) : limit(limit) {}
		//
		// 	void initialize() override
		// 	{
		// 		counter = 0;
		// 	}
		//
		// 	Status update() override
		// 	{
		// 		child->tick();
		//
		// 		if (limit > 0 && ++counter == limit) {
		// 			return Status::Success;
		// 		}
		//
		// 		return Status::Running;
		// 	}
		//
		// protected:
		// 	int limit;
		// 	int counter = 0;
		// };

		DECORATOR_CLASS(UntilSuccess)
			Status update() override
			{
				while (1) {
					auto status = child->tick();
		
					if (status == Status::Success) {
						return Status::Success;
					}
				}
			}
		};
		
		
		DECORATOR_CLASS(UntilFailure)
			Status update() override
			{
				while (1) {
					auto status = child->tick();
		
					if (status == Status::Failure) {
						return Status::Success;
					}
				}
			}
		};

	}
}
