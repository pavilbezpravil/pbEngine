#include "pch.h"
#include "BehaviorTree.h"
#include "Composite.h"

#include "Task.h"
#include "yaml-cpp/yaml.h"

namespace pbe
{
	namespace AI
	{
		static void SerializeNode(YAML::Emitter& out, const Node::Ptr& node)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "ID" << YAML::Value << node->ID;
			out << YAML::Key << "Type" << YAML::Value << Node::TypeToString(node->GetType());
			switch (node->GetType()) {
				case Node::Type::Leaf:
				{
					auto leaf = std::static_pointer_cast<Leaf>(node);
					out << YAML::Key << "TaskName" << YAML::Value << leaf->GetTask()->GetTaskName();
				}
				break;
				case Node::Type::Composite:
				{
					auto composite = std::static_pointer_cast<Composite>(node);
					out << YAML::Key << "CompositeType" << YAML::Value << composite->name;

					if (composite->hasChildren()) {
						out << YAML::Key << "ChildIDs" << YAML::Value;
						out << YAML::Flow;
						out << YAML::BeginSeq;
						for (auto& node : *composite) {
							out << node->ID;
						}
						out << YAML::EndSeq;
					}
				}
				break;
				case Node::Type::Decorator:
				{
					auto decorator = std::static_pointer_cast<Decorator>(node);
					out << YAML::Key << "DecoratorType" << YAML::Value << decorator->name;
					if (decorator->hasChild()) {
						out << YAML::Key << "ChildID" << YAML::Value << decorator->getChild()->ID;
					}
				}
				break;
			default: HZ_UNIMPLEMENTED();
			}

			out << YAML::EndMap;
		}

		static void DeserializeNodeInit(BehaviorTree& bt, YAML::Node& node)
		{
			int id = node["ID"].as<int>();
			Node::Type type = Node::StringToType(node["Type"].as<std::string>());

			Node::Ptr aiNode;
			
			switch (type) {
				case Node::Type::Leaf:
					aiNode = bt.AddNode<AI::Leaf>(id);
					break;
				case Node::Type::Composite:
					{
						std::string compositeType = node["CompositeType"].as<std::string>();
						if (compositeType == "Sequence") {
							aiNode = bt.AddNode<AI::Sequence>(id);
						} else if (compositeType == "Selector") {
							aiNode = bt.AddNode<AI::Selector>(id);
						}
						// else if (compositeType == "ParallelSequence") {
						// 	aiNode = bt.AddNode<AI::ParallelSequence>(id);
						// } else if (compositeType == "ParallelSelect") {
						// 	aiNode = bt.AddNode<AI::ParallelSequence>(id);
						// }
						else {
							HZ_UNIMPLEMENTED();
						}
					}
					
					break;
				case Node::Type::Decorator:
					{
						std::string decoratorType = node["DecoratorType"].as<std::string>();
							
						// todo:
						if (decoratorType == "Root") {
							aiNode = bt.AddNode<AI::Root>(id);
						} else {
							HZ_UNIMPLEMENTED()
						}
					}
					break;
				default: HZ_UNIMPLEMENTED();
			}
		}

		static void DeserializeNodeFill(BehaviorTree& bt, YAML::Node& node)
		{
			int id = node["ID"].as<int>();
			
			Node::Ptr aiNode = bt.GetNodeByID(id);

			switch (aiNode->GetType()) {
				case Node::Type::Leaf:
				{
					auto leaf = std::static_pointer_cast<Leaf>(aiNode);
					leaf->SetBlackboard(bt.getBlackboard());
					leaf->SetTask(TaskRegistry::Instance().Create(node["TaskName"].as<std::string>()));
				}
				break;
				case Node::Type::Composite:
				{
					Composite::Ptr composite = std::static_pointer_cast<Composite>(aiNode);
					auto& childIDs = node["ChildIDs"];
					if (childIDs) {
						for (auto& childID : childIDs) {
							composite->addChild(bt.GetNodeByID(childID.as<int>()));
						}
					}
				}
				break;
				case Node::Type::Decorator:
				{
					Decorator::Ptr decorator = std::static_pointer_cast<Decorator>(aiNode);
					auto& childID = node["ChildID"];
					if (childID) {
						decorator->setChild(bt.GetNodeByID(childID.as<int>()));
					}
				}
				break;
				default: HZ_UNIMPLEMENTED();
			}
		}
		
		void BehaviorTree::reset()
		{
			for (auto& node : nodes) {
				node->initialize();
			}
		}

		void BehaviorTree::Serialize(const std::string& filepath)
		{
			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "RootID" << YAML::Value << root->ID;
			out << YAML::Key << "Nodes";
			out << YAML::Value << YAML::BeginSeq;
			for (auto& node : nodes) {
				SerializeNode(out, node);
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;

			std::ofstream fout(filepath);
			fout << out.c_str();
		}

		bool BehaviorTree::Deserialize(const std::string& filepath)
		{
			std::ifstream stream(filepath);
			std::stringstream strStream;
			strStream << stream.rdbuf();

			YAML::Node data = YAML::Load(strStream.str());

			auto nodeNodes = data["Nodes"];
			if (nodeNodes) {
				for (auto & nodeNode : nodeNodes) {
					DeserializeNodeInit(*this, nodeNode);
				}
				for (auto& nodeNode : nodeNodes) {
					DeserializeNodeFill(*this, nodeNode);
				}
			}
			
			auto nodeRootID = data["RootID"];
			if (nodeRootID) {
				setRoot(GetNodeByID(nodeRootID.as<int>()));
			}
			
			return true;
		}

		void BehaviorTree::SetFilepath(const std::string& filepath)
		{
			this->filepath = filepath;
			int pos = filepath.find(".pbbt");
			this->filepathJson = filepath.substr(0, pos) + ".json";
		}

		// int BehaviorTree::GetUniqueNodeId()
		// {
		// 	return nextNodeId++;
		// }


	}
}
