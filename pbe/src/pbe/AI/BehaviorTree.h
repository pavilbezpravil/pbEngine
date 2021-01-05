#pragma once
#include "Node.h"

namespace pbe
{
	namespace AI
	{
		class BehaviorTree : public pbe::RefCounted //, public Node
		{
		public:
			BehaviorTree(const std::string& filepath) : blackboard(std::make_shared<Blackboard>())
			{
				SetFilepath(filepath);
			}
			// BehaviorTree(const Node::Ptr& rootNode) : BehaviorTree() { root = rootNode; }

			void reset();
			
			// Status update() { return root->tick(); }
			void update() { root->tick(); }

			void setRoot(const Node::Ptr& node) { root = node; }
			Node::Ptr& getRoot() { return root; }
			Blackboard::Ptr getBlackboard() const { return blackboard; }

			void Serialize(const std::string& filepath);
			bool Deserialize(const std::string& filepath);

			// int GetUniqueNodeId();

			template<typename T, typename ...Args>
			std::shared_ptr<T> AddNode(int id, Args&&... args)
			{
				auto ret = std::make_shared<T>(std::forward<Args>(args)...);
				ret->ID = id;
				nodes.push_back(ret);
				nodesMap[id] = ret;
				return ret;
			}

			void RemoveNode(const std::shared_ptr<Node>& node)
			{
				auto it = std::find(nodes.begin(), nodes.end(), node);
				if (it != nodes.end()) {
					nodes.erase(it);
				}
			}

			Node::Ptr& GetNodeByID(int id) { return nodesMap[id]; }
			const std::vector<Node::Ptr>& GetNodes() const { return nodes; }
			
			const std::string& GetFilepath() const { return filepath; }
			const std::string& GetFilepathJson() const { return filepathJson; }

		private:
			Node::Ptr root = nullptr;
			Blackboard::Ptr blackboard = nullptr;
			std::string filepath;
			std::string filepathJson;

			void SetFilepath(const std::string& filepath);

			// int nextNodeId = 0;
			std::vector<Node::Ptr> nodes;
			std::unordered_map<int, Node::Ptr> nodesMap;
		};
		
	}
}
