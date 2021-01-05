#pragma once
#include "Blackboard.h"

namespace YAML {
	class Emitter;
}

namespace pbe
{
	namespace AI
	{
		class Node
		{
		public:
			enum class Status
			{
				Invalid,
				Success,
				Failure,
				Running,
			};

			enum class Type
			{
				Leaf,
				Composite,
				Decorator,
			};

			static std::string TypeToString(Type type)
			{
				switch (type) {
					case Type::Leaf: return "Leaf";
					case Type::Decorator: return "Decorator";
					case Type::Composite: return "Composite";
				default: HZ_UNIMPLEMENTED();
				}
			}

			static Type StringToType(const std::string& str)
			{
				if (str == "Leaf") {
					return Type::Leaf;
				} else if (str == "Composite") {
					return Type::Composite;
				} else if (str == "Decorator") {
					return Type::Decorator;
				}
				HZ_UNIMPLEMENTED();
				return Type::Leaf;
			}

			virtual ~Node() {}

			virtual Status update() = 0;
			virtual void initialize() {}
			virtual void terminate(Status s) {}

			Status tick()
			{
				if (status != Status::Running) {
					initialize();
				}

				status = update();

				if (status != Status::Running) {
					terminate(status);
				}

				return status;
			}

			bool isSuccess() const { return status == Status::Success; }
			bool isFailure() const { return status == Status::Failure; }
			bool isRunning() const { return status == Status::Running; }
			bool isTerminated() const { return isSuccess() || isFailure(); }

			void reset() { status = Status::Invalid; }

			Type GetType() const { return type; }
			
			// virtual void Serialize(YAML::Emitter& out) = 0;
			// virtual void Deserialize(YAML::Emitter& out) = 0;

			using Ptr = std::shared_ptr<Node>;

			int ID = -1;
			std::string name;
			
		protected:
			Status status = Status::Invalid;
			Type type;
			
			Node(Type type) : type(type) {}
		};

		class Composite : public Node
		{
		public:
			Composite() : Node(Type::Composite), it(children.begin()) {}
			virtual ~Composite() {}

			void addChild(Node::Ptr child)
			{
				children.push_back(child);
			}
			
			void removeChild(Node::Ptr child)
			{
				auto it = std::find(children.begin(), children.end(), child);
				if (it != children.end()) {
					children.erase(it);
				}
			}
			bool hasChildren() const { return !children.empty(); }

			std::vector<Node::Ptr>::iterator begin() { return children.begin(); }
			std::vector<Node::Ptr>::iterator end() { return children.end(); }

			using Ptr = std::shared_ptr<Composite>;
			
		protected:
			std::vector<Node::Ptr> children;
			std::vector<Node::Ptr>::iterator it;
		};

		class Decorator : public Node
		{
		public:
			Decorator() : Node(Type::Decorator) {}
			virtual ~Decorator() {}

			void setChild(Node::Ptr node) { child = node; }
			Node::Ptr  getChild() { return child; }
			bool hasChild() const { return child != nullptr; }

			using Ptr = std::shared_ptr<Decorator>;
			
		protected:
			Node::Ptr child = nullptr;
		};

		class Root : public Decorator
		{
		public:
			Root() { name = "Root"; }
			
			Status update() { return hasChild() ? child->tick() : Status::Success; }
		};

		class Task
		{
		public:
			Task(const std::string& taskName) : taskName(taskName) {}
			
			virtual void init() {};
			virtual Node::Status tick(Blackboard::Ptr& blackboard) = 0;
			
			using Ptr = std::shared_ptr<Task>;

			const std::string& GetTaskName() const { return taskName; }
			
		private:
			std::string taskName;
		};
		
		class Leaf final : public Node
		{
		public:
			Leaf(Blackboard::Ptr blackboard = NULL, Task::Ptr pTask = NULL) : Node(Type::Leaf), blackboard(blackboard), pTask(pTask) {}

			Status update() override { return pTask ? pTask->tick(blackboard) : Status::Success; }

			void SetBlackboard(Blackboard::Ptr bb) { blackboard = bb; }
			void SetTask(Task::Ptr task) { pTask = task; name = pTask->GetTaskName();  }
			Task::Ptr& GetTask() { return pTask; }

			using Ptr = std::shared_ptr<Leaf>;
			
		protected:
			Blackboard::Ptr blackboard;
			Task::Ptr pTask;
		};
	}
}
