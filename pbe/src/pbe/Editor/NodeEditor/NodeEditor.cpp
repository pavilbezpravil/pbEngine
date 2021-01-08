#include "pch.h"

#include <filesystem>

#include "NodeEditor.h"
#include "utilities/builders.h"
#include "utilities/widgets.h"

#include <imgui_node_editor.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>


#include "pbe/AI/Composite.h"
#include "pbe/AI/Task.h"
#include "pbe/Core/Application.h"

namespace pbe
{

	static inline ImRect ImGui_GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
	{
		auto result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	namespace ed = ax::NodeEditor;
	namespace util = ax::NodeEditor::Utilities;

	using namespace ax;

	using ax::Widgets::IconType;

	int NodePanel::GetNextId()
	{
		return s_NextId++;
	}

	ed::LinkId NodePanel::GetNextLinkId()
	{
		return ed::LinkId(GetNextId());
	}

	void NodePanel::TouchNode(ed::NodeId id)
	{
		s_NodeTouchTime[id] = s_TouchTime;
	}

	float NodePanel::GetTouchProgress(ed::NodeId id)
	{
		auto it = s_NodeTouchTime.find(id);
		if (it != s_NodeTouchTime.end() && it->second > 0.0f)
			return (s_TouchTime - it->second) / s_TouchTime;
		else
			return 0.0f;
	}

	void NodePanel::UpdateTouch()
	{
		const auto deltaTime = ImGui::GetIO().DeltaTime;
		for (auto& entry : s_NodeTouchTime)
		{
			if (entry.second > 0.0f)
				entry.second -= deltaTime;
		}
	}

	Node* NodePanel::FindNode(ed::NodeId id)
	{
		for (auto& node : s_Nodes)
			if (node.ID == id)
				return &node;

		return nullptr;
	}

	Link* NodePanel::FindLink(ed::LinkId id)
	{
		for (auto& link : s_Links)
			if (link.ID == id)
				return &link;

		return nullptr;
	}

	Pin* NodePanel::FindPin(ed::PinId id)
	{
		if (!id)
			return nullptr;

		for (auto& node : s_Nodes) {
			for (auto& pin : node.Inputs)
				if (pin.ID == id)
					return &pin;

			for (auto& pin : node.Outputs)
				if (pin.ID == id)
					return &pin;
		}

		return nullptr;
	}

	bool NodePanel::IsPinLinked(ed::PinId id)
	{
		if (!id)
			return false;

		for (auto& link : s_Links)
			if (link.StartPinID == id || link.EndPinID == id)
				return true;

		return false;
	}

	bool NodePanel::CanCreateLink(Pin* a, Pin* b)
	{
		if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
			return false;

		return true;
	}

	void NodePanel::BuildNode(Node* node)
	{
		for (auto& input : node->Inputs)
		{
			input.Node = node;
			input.Kind = PinKind::Input;
		}

		for (auto& output : node->Outputs)
		{
			output.Node = node;
			output.Kind = PinKind::Output;
		}
	}

	void NodePanel::Reset()
	{
		Application_Finalize();
		
		s_NextId = 1;
		rootId = -1;
		s_NodeTouchTime.clear();
		s_Links.clear();
		s_Nodes.clear();
	}

	Link* NodePanel::CreateLink(Pin* startPin, Pin* endPin, bool considerBTree)
	{
		auto startAiNode = FindNode(startPin->Node->ID)->aiNode;
		auto endAiNode = FindNode(endPin->Node->ID)->aiNode;
		HZ_CORE_INFO("Try create link. Start node '{}', end node '{}'", startAiNode->name, endAiNode->name);
		if (endPin->Node->ID.Get() != rootId) {
			auto startType = startAiNode->GetType();
			if (startType == AI::Node::Type::Composite || startType == AI::Node::Type::Decorator) {
				if (considerBTree) {
					if (startType == AI::Node::Type::Composite) {
						auto composite = (AI::Composite*)startAiNode.get();
						composite->addChild(endAiNode);
					} else if (startType == AI::Node::Type::Decorator) {
						auto it = std::find_if(s_Links.begin(), s_Links.end(), [startPin](auto& link) { return link.StartPinID == startPin->ID; });
						if (it != s_Links.end()) {
							RemoveLink(it->ID);
						}

						auto composite = (AI::Decorator*)startAiNode.get();
						composite->setChild(endAiNode);
					} else {
						HZ_UNIMPLEMENTED();
					}
				}

				s_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
				HZ_CORE_INFO("Success create link. Start node '{}', end node '{}'", startAiNode->name, endAiNode->name);
				return &s_Links.back();
			}
		}
		
		return NULL;
	}

	void NodePanel::RemoveLink(ed::LinkId linkId)
	{
		auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
		
		if (id != s_Links.end()) {
			auto startPin = FindPin(id->StartPinID);
			auto endPin = FindPin(id->EndPinID);

			auto startAiNode = FindNode(startPin->Node->ID)->aiNode;
			auto endAiNode = FindNode(endPin->Node->ID)->aiNode;

			HZ_CORE_INFO("Try remove link. Start node '{}', end node '{}'", startAiNode->name, endAiNode->name);

			auto startType = startAiNode->GetType();
			if (startType == AI::Node::Type::Composite || startType == AI::Node::Type::Decorator) {
				if (startType == AI::Node::Type::Composite) {
					auto composite = (AI::Composite*)startAiNode.get();
					composite->removeChild(endAiNode);
				}
				if (startType == AI::Node::Type::Decorator) {
					auto composite = (AI::Decorator*)startAiNode.get();
					composite->setChild(NULL);
				}

				HZ_CORE_INFO("Success remove link. Start node '{}', end node '{}'", startAiNode->name, endAiNode->name);
			}

			s_Links.erase(id);
		}
	}

	Node* NodePanel::CreateNode(const AI::Node::Ptr& aiNode)
	{
		HZ_CORE_ASSERT(aiNode->ID != -1);
		
		Node* uiNode;
		s_Nodes.emplace_back(aiNode->ID);
		uiNode = &s_Nodes.back();
		uiNode->aiNode = aiNode;
		uiNode->Type = NodeType::Tree;

		return uiNode;
	}

	void NodePanel::RemoveNode(ed::NodeId nodeId)
	{
		auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
		if (id != s_Nodes.end()) {
			pContext->RemoveNode(id->aiNode);
			s_Nodes.erase(id);
		}
	}

	Node* NodePanel::SpawnDecoratorNode(const AI::Node::Ptr& aiNode)
	{
		Node* uiNode = CreateNode(aiNode);
		uiNode->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

		BuildNode(uiNode);

		return uiNode;
	}

	Node* NodePanel::SpawnCompositeNode(const AI::Node::Ptr& aiNode)
	{
		Node* uiNode = CreateNode(aiNode);
		uiNode->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
		uiNode->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

		BuildNode(uiNode);

		return uiNode;
	}

	Node* NodePanel::SpawnLeafNode(const AI::Node::Ptr& aiNode)
	{
		Node* uiNode = CreateNode(aiNode);
		uiNode->Inputs.emplace_back(GetNextId(), "", PinType::Flow);

		BuildNode(uiNode);

		return uiNode;
	}

	void NodePanel::AddRoot()
	{
		HZ_CORE_ASSERT(rootId == -1);
		rootId = GetNextId();
		auto aiNode = pContext->AddNode<AI::Root>(rootId);
		pContext->setRoot(aiNode);

		SpawnDecoratorNode(aiNode);
	}

	Node* NodePanel::SpawnComment()
	{
		s_Nodes.emplace_back(GetNextId());
		s_Nodes.back().Type = NodeType::Comment;
		s_Nodes.back().Size = ImVec2(300, 200);

		return &s_Nodes.back();
	}

	void NodePanel::BuildNodes()
	{
		for (auto& node : s_Nodes)
			BuildNode(&node);
	}

	void NodePanel::Application_Initialize(const std::string& filepath)
	{
		pContext = Ref<AI::BehaviorTree>::Create(filepath);
		
		ed::Config config;
		config.SettingsFile = pContext->GetFilepathJson().c_str();

		// config.UserPointer = this;
		//
		// config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
		// {
		// 	NodePanel* pThis = (NodePanel*)userPointer;
		// 	auto node = pThis->FindNode(nodeId);
		// 	if (!node)
		// 		return 0;
		//
		// 	if (data != nullptr) {
		// 		memcpy(data, node->State.data(), node->State.size());
		// 	}
		//
		// 	return node->State.size();
		// };
		//
		// config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
		// {
		// 	NodePanel* pThis = (NodePanel*)userPointer;
		// 	auto node = pThis->FindNode(nodeId);
		// 	if (!node)
		// 		return false;
		//
		// 	node->State.assign(data, size);
		//
		// 	pThis->TouchNode(nodeId);
		//
		// 	return true;
		// };

		m_Editor = ed::CreateEditor(&config);
		ed::SetCurrentEditor(m_Editor);

		ed::NavigateToContent();

		BuildNodes();
	}

	void NodePanel::Application_Finalize()
	{
		if (m_Editor) {
			ed::DestroyEditor(m_Editor);
			m_Editor = nullptr;
		}
		pContext = NULL;
	}

	bool NodePanel::Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

	ImColor NodePanel::GetIconColor(PinType type)
	{
		switch (type)
		{
		default:
		case PinType::Flow:     return ImColor(255, 255, 255);
		case PinType::Bool:     return ImColor(220, 48, 48);
		case PinType::Int:      return ImColor(68, 201, 156);
		case PinType::Float:    return ImColor(147, 226, 74);
		case PinType::String:   return ImColor(124, 21, 153);
		case PinType::Object:   return ImColor(51, 150, 215);
		case PinType::Function: return ImColor(218, 0, 183);
		case PinType::Delegate: return ImColor(255, 48, 48);
		}
	};

	void NodePanel::DrawPinIcon(const Pin& pin, bool connected, int alpha)
	{
		IconType iconType;
		ImColor  color = GetIconColor(pin.Type);
		color.Value.w = alpha / 255.0f;
		switch (pin.Type)
		{
		case PinType::Flow:     iconType = IconType::Flow;   break;
		case PinType::Bool:     iconType = IconType::Circle; break;
		case PinType::Int:      iconType = IconType::Circle; break;
		case PinType::Float:    iconType = IconType::Circle; break;
		case PinType::String:   iconType = IconType::Circle; break;
		case PinType::Object:   iconType = IconType::Circle; break;
		case PinType::Function: iconType = IconType::Circle; break;
		case PinType::Delegate: iconType = IconType::Square; break;
		default:
			return;
		}

		ax::Widgets::Icon(ImVec2(s_PinIconSize, s_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
	}

	void NodePanel::ExecuteGraphTraverse(AI::Node::Ptr node, int& n)
	{
		auto type = node->GetType();
		if (type == AI::Node::Type::Composite) {
			auto composite = std::static_pointer_cast<AI::Composite>(node);
			for (auto child : *composite) {
				ExecuteGraphTraverse(child, n);
			}
		} else if (type == AI::Node::Type::Decorator) {
			auto decorator = std::static_pointer_cast<AI::Decorator>(node);
			if (decorator->hasChild()) {
				ExecuteGraphTraverse(decorator->getChild(), n);
			}
		}
		FindNode(ed::NodeId{ (unsigned long long)node->ID })->executeNumber = n++;
	}

	void NodePanel::ShowStyleEditor(bool* show)
	{
		// if (!ImGui::Begin("Style", show))
		// {
		// 	ImGui::End();
		// 	return;
		// }
		//
		// auto paneWidth = ImGui::GetContentRegionAvailWidth();
		//
		// auto& editorStyle = ed::GetStyle();
		// ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
		// ImGui::TextUnformatted("Values");
		// ImGui::Spring();
		// if (ImGui::Button("Reset to defaults"))
		// 	editorStyle = ed::Style();
		// ImGui::EndHorizontal();
		// ImGui::Spacing();
		// ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
		// ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
		// ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
		// ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
		// ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
		// ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
		// ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
		// ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
		// //ImVec2  SourceDirection;
		// //ImVec2  TargetDirection;
		// ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
		// ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
		// ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
		// ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
		// //ImVec2  PivotAlignment;
		// //ImVec2  PivotSize;
		// //ImVec2  PivotScale;
		// //float   PinCorners;
		// //float   PinRadius;
		// //float   PinArrowSize;
		// //float   PinArrowWidth;
		// ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
		// ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);
		//
		// ImGui::Separator();
		//
		// static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_RGB;
		// ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
		// ImGui::TextUnformatted("Filter Colors");
		// ImGui::Spring();
		// ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_RGB);
		// ImGui::Spring(0);
		// ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_HSV);
		// ImGui::Spring(0);
		// ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_HEX);
		// ImGui::EndHorizontal();
		//
		// static ImGuiTextFilter filter;
		// filter.Draw("", paneWidth);
		//
		// ImGui::Spacing();
		//
		// ImGui::PushItemWidth(-160);
		// for (int i = 0; i < ed::StyleColor_Count; ++i)
		// {
		// 	auto name = ed::GetStyleColorName((ed::StyleColor)i);
		// 	if (!filter.PassFilter(name))
		// 		continue;
		//
		// 	ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
		// }
		// ImGui::PopItemWidth();
		//
		// ImGui::End();
	}

	void NodePanel::ShowLeftPane(float paneWidth)
	{
		auto& io = ImGui::GetIO();

		// ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));
		ImGui::BeginChild("Info", ImVec2(paneWidth, 0));

		paneWidth = ImGui::GetContentRegionAvailWidth();

		static bool showStyleEditor = false;
		// ImGui::BeginHorizontal("Style Editor", ImVec2(paneWidth, 0));
		// ImGui::Spring(0.0f, 0.0f);
		if (ImGui::Button("Zoom to Content"))
			ed::NavigateToContent();
		ImGui::SameLine();
		// ImGui::Spring(0.0f);
		if (ImGui::Button("Show Flow")) {
			for (auto& link : s_Links)
				ed::Flow(link.ID);
		}
		ImGui::SameLine();
		// ImGui::Spring();
		if (ImGui::Button("Edit Style"))
			showStyleEditor = true;
		// ImGui::EndHorizontal();

		if (showStyleEditor)
			ShowStyleEditor(&showStyleEditor);

		std::vector<ed::NodeId> selectedNodes;
		std::vector<ed::LinkId> selectedLinks;
		selectedNodes.resize(ed::GetSelectedObjectCount());
		selectedLinks.resize(ed::GetSelectedObjectCount());

		int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
		int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

		selectedNodes.resize(nodeCount);
		selectedLinks.resize(linkCount);

		ShowBehaviorTreeInfo();
		
		ImGui::GetWindowDrawList()->AddRectFilled(
			ImGui::GetCursorScreenPos(),
			ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
			ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
		ImGui::Spacing(); ImGui::SameLine();
		ImGui::TextUnformatted("Nodes");
		ImGui::Indent();
		for (auto& node : s_Nodes) {
			ImGui::PushID(node.ID.AsPointer());
			auto start = ImGui::GetCursorScreenPos();

			if (const auto progress = GetTouchProgress(node.ID)) {
				ImGui::GetWindowDrawList()->AddLine(
					start + ImVec2(-8, 0),
					start + ImVec2(-8, ImGui::GetTextLineHeight()),
					IM_COL32(255, 0, 0, 255 - (int)(255 * progress)), 4.0f);
			}

			bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), node.ID) != selectedNodes.end();
			if (ImGui::Selectable((node.aiNode->name + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer()))).c_str(), &isSelected)) {
				if (io.KeyCtrl) {
					if (isSelected) {
						ed::SelectNode(node.ID, true);
					} else {
						ed::DeselectNode(node.ID);
					}
				} else {
					ed::SelectNode(node.ID, false);
				}

				ed::NavigateToSelection();
			}
			ImGui::SetItemAllowOverlap();

			if (ImGui::IsItemHovered() && !node.State.empty())
				ImGui::SetTooltip("State: %s", node.State.c_str());

			auto id = std::string("(") + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer())) + ")";
			auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);
			ImGui::SameLine();
			ImGui::Text(id.c_str(), nullptr);

			ImGui::SetItemAllowOverlap();
			// ImGui::SameLine(ImGui::GetWindowWidth());
			// ImGui::SameLine();

			if (node.SavedState.empty()) {
				ImGui::SameLine();
				if (ImGui::Button("save"))
					node.SavedState = node.State;
			}
			else {
				// ImGui::Dummy(ImVec2((float)saveIconWidth, (float)saveIconHeight));
			}

			// ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::SetItemAllowOverlap();
			if (!node.SavedState.empty()) {
				ImGui::SameLine();
				if (ImGui::Button("restore")) {
					node.State = node.SavedState;
					ed::RestoreNodeState(node.ID);
					node.SavedState.clear();
				}
			}
			else {
				// ImGui::Dummy(ImVec2((float)restoreIconWidth, (float)restoreIconHeight));
			}

			// ImGui::SameLine(0, 0);
			ImGui::SetItemAllowOverlap();
			// ImGui::Dummy(ImVec2(0, (float)restoreIconHeight));

			ImGui::PopID();
		}
		ImGui::Unindent();

		static int changeCount = 0;

		ImGui::GetWindowDrawList()->AddRectFilled(
			ImGui::GetCursorScreenPos(),
			ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
			ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
		ImGui::Spacing(); ImGui::SameLine();
		ImGui::TextUnformatted("Selection");

		// ImGui::BeginHorizontal("Selection Stats", ImVec2(paneWidth, 0));
		ImGui::Text("Changed %d time%s", changeCount, changeCount > 1 ? "s" : "");
		// ImGui::Spring();
		ImGui::SameLine();
		if (ImGui::Button("Deselect All"))
			ed::ClearSelection();
		// ImGui::EndHorizontal();
		ImGui::Indent();
		for (int i = 0; i < nodeCount; ++i) ImGui::Text("Node (%p)", selectedNodes[i].AsPointer());
		for (int i = 0; i < linkCount; ++i) ImGui::Text("Link (%p)", selectedLinks[i].AsPointer());
		ImGui::Unindent();

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
			for (auto& link : s_Links)
				ed::Flow(link.ID);

		if (ed::HasSelectionChanged())
			++changeCount;

		ImGui::Spacing();
		if (ImGui::Checkbox("execute tree", &executeTree)) {
			pContext->reset();
		}

		ImGui::EndChild();
	}

	void NodePanel::ShowBehaviorTreeInfo()
	{
		// ImGui::GetWindowDrawList()->AddRectFilled(
		// 	ImGui::GetCursorScreenPos(),
		// 	ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		// 	ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
		// ImGui::Spacing(); ImGui::SameLine();
		ImGui::Text("Behavior Tree");
		ImGui::Indent();
		ImGui::Text("nodes: %d", pContext->GetNodes().size());
		ImGui::Unindent();
		
	}

	void NodePanel::Application_Frame()
	{
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl-N")) {
					auto& app = Application::Get();
					std::string filepath = app.SaveFile("Behavior Tree (*.pbbt)\0*.pbbt\0");
					if (!filepath.empty()) {
						NewContext(filepath);
						AddRoot();
					}
				}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					// OpenScene();
					auto& app = Application::Get();
					std::string filepath = app.OpenFile("Behavior Tree (*.pbbt)\0*.pbbt\0");
					if (!filepath.empty()) {
						OpenTree(filepath);
					}
				}
					
				ImGui::Separator();
				if (ImGui::MenuItem("Save", "Ctrl+S", false, pContext)) {
					SaveTree();
					// SaveScene();
				}
					
				// if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
				// 	// SaveSceneAs();
				// 	auto& app = Application::Get();
				// 	std::string filepath = app.SaveFile("Behavior Tree (*.pbbt)\0*.pbbt\0");
				// 	if (!filepath.empty()) {
				// 		pContext->filepath = filepath;
				// 		SaveTree(filepath);
				// 	}
				// }	
				//
				// ImGui::Separator();
				// if (ImGui::MenuItem("Exit"))
				// 	p_open = false;
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// todo:
		if (!pContext) {
			// NewContext("tmp.pbbt");
			// AddRoot();
			return;
		}

		if (executeTree) {
			// todo:
			pContext->update(NULL);
		}
		
		UpdateTouch();

		auto& io = ImGui::GetIO();

		ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

		ed::SetCurrentEditor(m_Editor);

		static ed::NodeId contextNodeId = 0;
		static ed::LinkId contextLinkId = 0;
		static ed::PinId  contextPinId = 0;
		static bool createNewNode = false;
		static Pin* newNodeLinkPin = nullptr;
		static Pin* newLinkPin = nullptr;

		static float leftPaneWidth = 400.0f;
		static float rightPaneWidth = 800.0f;
		Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

		ShowLeftPane(leftPaneWidth - 4.0f);

		ImGui::SameLine(0.0f, 12.0f);

		ed::Begin("Node editor");
		{
			auto cursorTopLeft = ImGui::GetCursorScreenPos();

			for (auto& node : s_Nodes) {
				node.executeNumber = -1;
			}

			int n = 0;
			ExecuteGraphTraverse(pContext->getRoot(), n);
			
			for (auto& node : s_Nodes) {
				if (node.Type != NodeType::Tree)
					continue;

				const float rounding = 5.0f;
				const float padding = 12.0f;

				const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

				ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
				ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
				ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
				ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

				ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
				ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
				ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
				ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
				ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
				ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
				ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
				ed::BeginNode(node.ID);

				// ImGui::BeginVertical(node.ID.AsPointer());
				// ImGui::BeginHorizontal("inputs");
				// ImGui::Spring(0, padding * 2);

				ImRect inputsRect;
				int inputAlpha = 200;
				if (!node.Inputs.empty()) {
					auto& pin = node.Inputs[0];
					// ImGui::Dummy(ImVec2(0, padding));
					// ImGui::Spring(1, 0);
					// float xSize =  ImGui::CalcTextSize("     ").x;
					// ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ed::GetNodeSize(node.ID).x - xSize) * 0.5f);
					ImGui::Text("Input");
					inputsRect = ImGui_GetItemRect();

					ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
					ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
					ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
					ed::BeginPin(pin.ID, ed::PinKind::Input);
					ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
					ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
					ed::EndPin();
					ed::PopStyleVar(3);

					if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
						inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
				} else {
					ImGui::Dummy(ImVec2(0, padding));
				}
					
				// ImGui::Spring(0, padding * 2);
				// ImGui::EndHorizontal();
				//
				// ImGui::BeginHorizontal("content_frame");
				// ImGui::Spring(1, padding);
				//
				// ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
				// ImGui::Dummy(ImVec2(160, 0));
				// ImGui::Spring(1);
				ImGui::TextUnformatted(node.aiNode->name.c_str());
				ImGui::SameLine();
				ImGui::Text("%d", node.executeNumber);
				// ImGui::Spring(1);
				// ImGui::EndVertical();
				auto contentRect = ImGui_GetItemRect();
				//
				// ImGui::Spring(1, padding);
				// ImGui::EndHorizontal();
				//
				// ImGui::BeginHorizontal("outputs");
				// ImGui::Spring(0, padding * 2);

				ImRect outputsRect;
				int outputAlpha = 200;
				if (!node.Outputs.empty()) {
					auto& pin = node.Outputs[0];
					// ImGui::Dummy(ImVec2(0, padding));
					// ImGui::Spring(1, 0);
					ImGui::Text("Output");
					outputsRect = ImGui_GetItemRect();

					ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
					ed::BeginPin(pin.ID, ed::PinKind::Output);
					ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
					ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
					ed::EndPin();
					ed::PopStyleVar();

					if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
						outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
				} else {
					ImGui::Dummy(ImVec2(0, padding));
				}
					
				// ImGui::Spring(0, padding * 2);
				// ImGui::EndHorizontal();
				//
				// ImGui::EndVertical();

				ed::EndNode();
				ed::PopStyleVar(7);
				ed::PopStyleColor(4);

				auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

				drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
					IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
				drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
					IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
				drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
					IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
				drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
					IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
				drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
				drawList->AddRect(
					contentRect.GetTL(),
					contentRect.GetBR(),
					IM_COL32(48, 128, 255, 100), 0.0f);
			}

			for (auto& node : s_Nodes) {
				if (node.Type != NodeType::Comment)
					continue;
			
				const float commentAlpha = 0.75f;
			
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
				ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
				ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
				ed::BeginNode(node.ID);
				ImGui::PushID(node.ID.AsPointer());
				// ImGui::BeginVertical("content");
				// ImGui::BeginHorizontal("horizontal");
				// ImGui::Spring(1);
				ImGui::TextUnformatted("Some comment");
				// ImGui::Spring(1);
				// ImGui::EndHorizontal();
				ed::Group(node.Size);
				// ImGui::EndVertical();
				ImGui::PopID();
				ed::EndNode();
				ed::PopStyleColor(2);
				ImGui::PopStyleVar();
			
				if (ed::BeginGroupHint(node.ID)) {
					auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);
					auto min = ed::GetGroupMin();
			
					ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
					ImGui::BeginGroup();
					ImGui::TextUnformatted("Some comment");
					ImGui::EndGroup();
			
					auto drawList = ed::GetHintBackgroundDrawList();
			
					auto hintBounds = ImGui_GetItemRect();
					auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);
			
					drawList->AddRectFilled(
						hintFrameBounds.GetTL(),
						hintFrameBounds.GetBR(),
						IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);
			
					drawList->AddRect(
						hintFrameBounds.GetTL(),
						hintFrameBounds.GetBR(),
						IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);
				}
				ed::EndGroupHint();
			}

			for (auto& link : s_Links) {
				ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);
			}

			if (!createNewNode) {
				if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f)) {
					auto showLabel = [](const char* label, ImColor color) {
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
						auto size = ImGui::CalcTextSize(label);

						auto padding = ImGui::GetStyle().FramePadding;
						auto spacing = ImGui::GetStyle().ItemSpacing;

						ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

						auto rectMin = ImGui::GetCursorScreenPos() - padding;
						auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

						auto drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
						ImGui::TextUnformatted(label);
					};

					ed::PinId startPinId = 0, endPinId = 0;
					if (ed::QueryNewLink(&startPinId, &endPinId)) {
						auto startPin = FindPin(startPinId);
						auto endPin = FindPin(endPinId);

						newLinkPin = startPin ? startPin : endPin;

						if (startPin->Kind == PinKind::Input) {
							std::swap(startPin, endPin);
							std::swap(startPinId, endPinId);
						}

						if (startPin && endPin)
						{
							if (endPin == startPin)
							{
								ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
							}
							else if (endPin->Kind == startPin->Kind)
							{
								showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
								ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
							}
							else if (endPin->Type != startPin->Type)
							{
								showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
								ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
							}
							else
							{
								showLabel("+ Create Link", ImColor(32, 45, 32, 180));
								if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
								{
									auto link = CreateLink(startPin, endPin);
									link->Color = GetIconColor(startPin->Type);
								}
							}
						}
					}

					ed::PinId pinId = 0;
					if (ed::QueryNewNode(&pinId)) {
						newLinkPin = FindPin(pinId);
						if (newLinkPin)
							showLabel("+ Create Node", ImColor(32, 45, 32, 180));

						if (ed::AcceptNewItem()) {
							createNewNode = true;
							newNodeLinkPin = FindPin(pinId);
							newLinkPin = nullptr;
							ed::Suspend();
							ImGui::OpenPopup("Create New Node");
							ed::Resume();
						}
					}
				} else {
					newLinkPin = nullptr;
				}
				ed::EndCreate();

				if (ed::BeginDelete()) {
					ed::LinkId linkId = 0;
					while (ed::QueryDeletedLink(&linkId)) {
						if (ed::AcceptDeletedItem()) {
							RemoveLink(linkId);
						}
					}

					ed::NodeId nodeId = 0;
					while (ed::QueryDeletedNode(&nodeId)) {
						bool canDelete = nodeId.Get() != rootId;
						if (canDelete) {
							if (ed::AcceptDeletedItem()) {
								RemoveNode(nodeId);
							}
						} else {
							ed::RejectDeletedItem();
						}
					}
				}
				ed::EndDelete();
			}

			ImGui::SetCursorScreenPos(cursorTopLeft);
		}

# if 1
		auto openPopupPosition = ImGui::GetMousePos();
		ed::Suspend();
		if (ed::ShowNodeContextMenu(&contextNodeId))
			ImGui::OpenPopup("Node Context Menu");
		else if (ed::ShowPinContextMenu(&contextPinId))
			ImGui::OpenPopup("Pin Context Menu");
		else if (ed::ShowLinkContextMenu(&contextLinkId))
			ImGui::OpenPopup("Link Context Menu");
		else if (ed::ShowBackgroundContextMenu())
		{
			ImGui::OpenPopup("Create New Node");
			newNodeLinkPin = nullptr;
		}
		ed::Resume();

		ed::Suspend();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
		if (ImGui::BeginPopup("Node Context Menu")) {
			auto node = FindNode(contextNodeId);

			ImGui::TextUnformatted("Node Context Menu");
			ImGui::Separator();
			if (node) {
				ImGui::Text("ID: %p", node->ID.AsPointer());
				ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
				ImGui::Text("Inputs: %d", (int)node->Inputs.size());
				ImGui::Text("Outputs: %d", (int)node->Outputs.size());
			} else {
				ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
			}
				
			ImGui::Separator();
			if (ImGui::MenuItem("Delete"))
				ed::DeleteNode(contextNodeId);
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Pin Context Menu"))
		{
			auto pin = FindPin(contextPinId);

			ImGui::TextUnformatted("Pin Context Menu");
			ImGui::Separator();
			if (pin)
			{
				ImGui::Text("ID: %p", pin->ID.AsPointer());
				if (pin->Node)
					ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
				else
					ImGui::Text("Node: %s", "<none>");
			}
			else
				ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Link Context Menu"))
		{
			auto link = FindLink(contextLinkId);

			ImGui::TextUnformatted("Link Context Menu");
			ImGui::Separator();
			if (link)
			{
				ImGui::Text("ID: %p", link->ID.AsPointer());
				ImGui::Text("From: %p", link->StartPinID.AsPointer());
				ImGui::Text("To: %p", link->EndPinID.AsPointer());
			}
			else
				ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
			ImGui::Separator();
			if (ImGui::MenuItem("Delete"))
				ed::DeleteLink(contextLinkId);
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Create New Node"))
		{
			auto newNodePostion = openPopupPosition;

			Node* node = nullptr;
			{
				if (ImGui::MenuItem("Comment"))
					node = SpawnComment();
			}
			{
				ImGui::Separator();
				if (ImGui::MenuItem("Sequence")) {
					auto aiNode = pContext->AddNode<AI::Sequence>(GetNextId());
					node = SpawnCompositeNode(aiNode);
				}
				if (ImGui::MenuItem("Selector")) {
					auto aiNode = pContext->AddNode<AI::Selector>(GetNextId());
					node = SpawnCompositeNode(aiNode);
				}
			}
			{
				ImGui::Separator();
				auto& TaskRegistry = AI::TaskRegistry::Instance();
				for (auto& entry : TaskRegistry.GetRegistryMap()) {
					if (ImGui::MenuItem(TaskRegistry.GetRegistryIDByName(entry.first))) {
						auto aiNode = pContext->AddNode<AI::Leaf>(GetNextId());
						aiNode->SetTask(entry.second());
						aiNode->SetBlackboard(pContext->getBlackboard());

						node = SpawnLeafNode(aiNode);
					}
				}
			}

			if (node) {
				BuildNodes();

				createNewNode = false;

				ed::SetNodePosition(node->ID, newNodePostion);

				if (auto startPin = newNodeLinkPin)
				{
					auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

					for (auto& pin : pins)
					{
						if (CanCreateLink(startPin, &pin))
						{
							auto endPin = &pin;
							if (startPin->Kind == PinKind::Input)
								std::swap(startPin, endPin);

							Link* link = CreateLink(startPin, endPin);
							link->Color = GetIconColor(startPin->Type);

							break;
						}
					}
				}
			}

			ImGui::EndPopup();
		} else {
			createNewNode = false;
		}
			
		ImGui::PopStyleVar();
		ed::Resume();
# endif

		ed::End();
	}

	void NodePanel::NewContext(const std::string& filepath)
	{
		Reset();
		Application_Initialize(filepath);
	}

	void NodePanel::SaveTree()
	{
		pContext->Serialize(pContext->GetFilepath());
		// std::filesystem::copy();
	}

	void NodePanel::OpenTree(const std::string& filepath)
	{
		NewContext(filepath);
		
		pContext->Deserialize(filepath);
		rootId = pContext->getRoot()->ID;
		for (auto& node : pContext->GetNodes()) {
			s_NextId = std::max(s_NextId, node->ID);
		}
		++s_NextId;

		for (auto& node : pContext->GetNodes()) {
			switch (node->GetType()) {
			case AI::Node::Type::Leaf: SpawnLeafNode(node); break;
			case AI::Node::Type::Composite: SpawnCompositeNode(node); break;
			case AI::Node::Type::Decorator: SpawnDecoratorNode(node); break;
			default:;
			}
		}

		BuildNodes();

		for (auto& node : pContext->GetNodes()) {
			switch (node->GetType()) {
				case AI::Node::Type::Leaf: break;
				case AI::Node::Type::Composite:
				{
					AI::Composite::Ptr composite = std::static_pointer_cast<AI::Composite>(node);
					for (auto& child : *composite) {
						CreateLink(&FindNode(composite->ID)->Outputs[0], &FindNode(child->ID)->Inputs[0], false);
					}
				}
				break;
				case AI::Node::Type::Decorator:
				{
					AI::Decorator::Ptr decorator = std::static_pointer_cast<AI::Decorator>(node);
					if (decorator->hasChild()) {
						CreateLink(&FindNode(decorator->ID)->Outputs[0], &FindNode(decorator->getChild()->ID)->Inputs[0], false);
					}
				}
				break;
				default: HZ_UNIMPLEMENTED();
			}
		}
	}
}
