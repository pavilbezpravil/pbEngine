#pragma once
#include "Node.h"

namespace pbe
{
	namespace AI
	{
		class Succeeder : public Decorator
		{
		public:
			Status update() override
			{
				child->tick();
				return Status::Success;
			}
		};

		
		class Failer : public Decorator
		{
		public:
			Status update() override
			{
				child->tick();
				return Status::Failure;
			}
		};

		
		class Inverter : public Decorator
		{
		public:
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

		
		class Repeater : public Decorator
		{
		public:
			Repeater(int limit = 0) : limit(limit) {}

			void initialize() override
			{
				counter = 0;
			}

			Status update() override
			{
				child->tick();

				if (limit > 0 && ++counter == limit) {
					return Status::Success;
				}

				return Status::Running;
			}

		protected:
			int limit;
			int counter = 0;
		};

		
		class UntilSuccess : public Decorator
		{
		public:
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

		
		class UntilFailure : public Decorator
		{
		public:
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
