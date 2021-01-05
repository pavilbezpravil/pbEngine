#pragma once

#include "utilities/builders.h"
#include "utilities/widgets.h"
#include "pbe/AI/BehaviorTree.h"

#include <imgui-node-editor/imgui_node_editor.h>
#include <map>

#include <string>
#include <variant>
#include <vector>

namespace pbe {

	namespace ed = ax::NodeEditor;
	namespace util = ax::NodeEditor::Utilities;

	enum class PinType
	{
		Flow,
		Bool,
		Int,
		Float,
		String,
		Object,
		Function,
		Delegate,
	};

	enum class PinKind
	{
		Output,
		Input
	};

	enum class NodeType
	{
		Blueprint,
		Simple,
		Tree,
		Comment,
	};

	struct Node;

	struct Pin
	{
		ed::PinId   ID;
		Node*		Node = NULL;
		std::string Name;
		PinType     Type;
		PinKind     Kind = PinKind::Input;

		Pin(int id, const char* name, PinType type) :
			ID(id), Name(name), Type(type)
		{
		}
	};

	struct Node
	{
		AI::Node::Ptr aiNode = NULL;
		
		ed::NodeId ID;
		std::vector<Pin> Inputs;
		std::vector<Pin> Outputs;
		ImColor Color;
		NodeType Type;
		ImVec2 Size;
		
		int executeNumber = -1;

		std::string State;
		std::string SavedState;

		Node(int id, ImColor color = ImColor(255, 255, 255)) :
			ID(id), Color(color), Type(NodeType::Blueprint), Size(0, 0)
		{
		}
	};

	struct Link
	{
		ed::LinkId ID;

		ed::PinId StartPinID;
		ed::PinId EndPinID;

		ImColor Color;

		Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
			ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
		{
		}
	};

	struct NodeIdLess
	{
		bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
		{
			return lhs.AsPointer() < rhs.AsPointer();
		}
	};

	class NodePanel
	{
	public:

		int GetNextId();
		ed::LinkId GetNextLinkId();

		void TouchNode(ed::NodeId id);
		float GetTouchProgress(ed::NodeId id);
		void UpdateTouch();

		Node* FindNode(ed::NodeId id);
		Link* FindLink(ed::LinkId id);
		Pin* FindPin(ed::PinId id);

		bool IsPinLinked(ed::PinId id);
		bool CanCreateLink(Pin* a, Pin* b);

		void BuildNode(Node* node);
		void BuildNodes();

		void Application_Initialize(const std::string& filepath);
		void Application_Finalize();

		void ShowStyleEditor(bool* show = nullptr);
		void ShowLeftPane(float paneWidth);
		void ShowBehaviorTreeInfo();
		void Application_Frame();

	private:

		void NewContext(const std::string& filepath);
		void SaveTree();
		void OpenTree(const std::string& filepath);

		pbe::Ref<pbe::AI::BehaviorTree> pContext;

		ed::EditorContext* m_Editor = nullptr;

		const int            s_PinIconSize = 24;
		std::vector<Node>    s_Nodes;
		std::vector<Link>    s_Links;

		const float          s_TouchTime = 1.0f;
		std::map<ed::NodeId, float, NodeIdLess> s_NodeTouchTime;

		int s_NextId = 1;
		
		int rootId = -1;

		bool executeTree = false;

		void Reset();
		
		Link* CreateLink(Pin* startPin, Pin* endPin, bool considerBTree = true);
		void RemoveLink(ed::LinkId linkId);
		
		Node* CreateNode(const AI::Node::Ptr& aiNode);
		void RemoveNode(ed::NodeId nodeId);
		
		Node* SpawnDecoratorNode(const AI::Node::Ptr& aiNode);
		Node* SpawnCompositeNode(const AI::Node::Ptr& aiNode);
		Node* SpawnLeafNode(const AI::Node::Ptr& aiNode);

		void AddRoot();

		Node* SpawnComment();

		bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
		ImColor GetIconColor(PinType type);
		void DrawPinIcon(const Pin& pin, bool connected, int alpha);

		void ExecuteGraphTraverse(AI::Node::Ptr node, int& n);
	};


}
