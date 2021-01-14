#include "pch.h"
#include "SceneHierarchyPanel.h"

#include <imgui.h>

#include "pbe/Core/Application.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Script/ScriptEngine.h"
// #include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pbe/Core/Utility.h"

namespace pbe {

	// glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix);

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
		: m_Context(context)
	{
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
		if (m_SelectionContext && false)
		{
			// Try and find same entity in new scene
			auto& entityMap = m_Context->GetEntityMap();
			UUID selectedEntityID = m_SelectionContext.GetUUID();
			if (entityMap.find(selectedEntityID) != entityMap.end())
				m_SelectionContext = entityMap.at(selectedEntityID);
		}
	}

	void SceneHierarchyPanel::SetTransformSpace(Space space)
	{
		m_TransSpace = space;
	}

	void SceneHierarchyPanel::SetSelected(Entity entity)
	{
		m_SelectionContext = entity;
	}

	// template<typename T, typename F = std::function<void(Entity&, T&)>>
	// template<typename T, typename F = void (*)(Entity&, T&)>
	template<typename T>
	static void AddComponent(Entity m_SelectionContext, const char* name, void (*f)(Entity&, T&) = {})
	{
		if (!m_SelectionContext.HasComponent<T>()) {
			if (ImGui::Button(name)) {
				auto& c = m_SelectionContext.AddComponent<T>();
				if (f) {
					f(m_SelectionContext, c);
				}
				ImGui::CloseCurrentPopup();
			}
		}
	}
	
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		if (m_Context) {
			uint32_t entityCount = 0, meshCount = 0;
			m_Context->m_Registry.each([&](auto entity) {
				Entity e(entity, m_Context.Raw());
				if (e.HasComponent<IDComponent>()) {
					if (!e.GetComponent<TransformComponent>().HasParent()) {
						DrawEntityNode(e);
					}
				}
			});

			if (m_WasTransAttach) {
				Entity attachedEntity = m_Context->GetEntityMap().at(m_TransAttachInfo.attached);
				auto& attachedTrans = attachedEntity.GetComponent<TransformComponent>();
				if (m_TransAttachInfo.parent == UUID_INVALID) { // dettach
					attachedTrans.DettachFromParent();
				} else { // attach
					attachedTrans.Attach(m_TransAttachInfo.parent);
					Entity parentEntity = m_Context->GetEntityMap().at(m_TransAttachInfo.parent);
					HZ_CORE_ASSERT(attachedTrans.ParentUUID == parentEntity.GetUUID());
					HZ_CORE_ASSERT(vector_find(parentEntity.GetComponent<TransformComponent>().ChildUUIDs, m_TransAttachInfo.attached) != -1);
				}

				m_WasTransAttach = false;
			}

			if (m_WasDeleteEntity) {
				Entity deletedEntity = m_Context->GetEntityMap().at(m_DeleteEntityInfo.deletedEntity);

				m_EntityDeletedCallback(deletedEntity);

				if (m_DeleteEntityInfo.hierDelete) {
					m_Context->DestroyEntityHierarchy(deletedEntity);
				} else {
					m_Context->DestroyEntity(deletedEntity);
				}
				
				if (deletedEntity == m_SelectionContext)
					m_SelectionContext = {};

				m_WasDeleteEntity = false;
			}

			if (ImGui::BeginPopupContextWindow(0, 1, false)) {
				if (ImGui::MenuItem("Create Empty Entity")) {
					m_Context->CreateEntity("Empty Entity");
				}
				ImGui::EndPopup();
			}

			ImGui::End();

			ImGui::Begin("Properties");

			if (m_SelectionContext) {
				DrawComponents(m_SelectionContext);

				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("AddComponentPanel");

				if (ImGui::BeginPopup("AddComponentPanel")) {
					AddComponent<CameraComponent>(m_SelectionContext ,"Camera");
					AddComponent<MeshComponent>(m_SelectionContext ,"Mesh");
					AddComponent<ScriptComponent>(m_SelectionContext ,"Script");
					AddComponent<DirectionLightComponent>(m_SelectionContext ,"Direction Light");
					AddComponent<PointLightComponent>(m_SelectionContext ,"Point Light");
					AddComponent<SpotLightComponent>(m_SelectionContext ,"Spot Light");
					AddComponent<BoxColliderComponent>(m_SelectionContext ,"Box Collider");
					AddComponent<SphereColliderComponent>(m_SelectionContext ,"Sphere Collider");
					AddComponent<RigidbodyComponent>(m_SelectionContext ,"Rigidbody");
					AddComponent<AIControllerComponent>(m_SelectionContext ,"AI Controller", [](Entity& e, AIControllerComponent& c) { c.AIController = Ref<AI::Controller>::Create(e); });
					AddComponent<SoundSourceComponent>(m_SelectionContext ,"Sound Source");

					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		const char* name = "Unnamed Entity";
		if (entity.HasComponent<TagComponent>())
			name = entity.GetComponent<TagComponent>().Tag.c_str();

		const auto& trans = entity.GetComponent<TransformComponent>();

		ImGuiTreeNodeFlags node_flags = (entity == m_SelectionContext ? ImGuiTreeNodeFlags_Selected : 0)
										| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
										| ImGuiTreeNodeFlags_SpanFullWidth
										| (trans.HasChilds() ? 0 : ImGuiTreeNodeFlags_Leaf);

		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, node_flags, name);
		if (ImGui::IsItemClicked()) {
			m_SelectionContext = entity;
			if (m_SelectionChangedCallback)
				m_SelectionChangedCallback(m_SelectionContext);
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
			ImGui::SetDragDropPayload("EDITOR_ENTITY_TRANS_ATTACH", &entity.GetUUID(), sizeof(UUID));
			ImGui::Text("Attach to");
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EDITOR_ENTITY_TRANS_ATTACH")) {
				IM_ASSERT(payload->DataSize == sizeof(UUID));
				UUID attachedUUID = *(const UUID*)payload->Data;

				if (0) {
					HZ_CORE_INFO("WasTransAttached!");
					Entity attachedEntity = m_Context->GetEntityMap().at(attachedUUID);
					Entity parentEntity = m_Context->GetEntityMap().at(entity.GetUUID());
					HZ_CORE_INFO("Attached entity name: {}", attachedEntity.GetComponent<TagComponent>().Tag);
					HZ_CORE_INFO("Parent entity name: {}", parentEntity.GetComponent<TagComponent>().Tag);
				}

				HZ_CORE_ASSERT(!m_WasTransAttach);
				m_WasTransAttach = true;

				m_TransAttachInfo.attached = attachedUUID;
				m_TransAttachInfo.parent = entity.GetUUID();
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginPopupContextItem()) {
			bool wasDelete = false;
			
			if (ImGui::MenuItem("Delete")) {
				wasDelete = true;
				m_DeleteEntityInfo.hierDelete = false;
			}
			if (ImGui::MenuItem("Delete Hierarchy")) {
				wasDelete = true;
				m_DeleteEntityInfo.hierDelete = true;
			}

			if (wasDelete) {
				HZ_CORE_ASSERT(m_WasDeleteEntity == false);
				m_DeleteEntityInfo.deletedEntity = entity.GetUUID();
				m_WasDeleteEntity = true;
			}

			if (ImGui::MenuItem("Dettach")) {
				HZ_CORE_ASSERT(!m_WasTransAttach);
				m_WasTransAttach = true;

				m_TransAttachInfo.attached = entity.GetUUID();
				m_TransAttachInfo.parent = UUID_INVALID;
			}
				
			ImGui::EndPopup();
		}

		if (opened) {
			for (UUID uuid : trans.ChildUUIDs) {
				DrawEntityNode(m_Context->GetEntityMap().at(uuid));
			}

			ImGui::TreePop();
		}
	}

	// void SceneHierarchyPanel::DrawMeshNode(const Ref<Mesh>& mesh, uint32_t& imguiMeshID)
	// {
	// 	static char imguiName[128];
	// 	memset(imguiName, 0, 128);
	// 	sprintf(imguiName, "Mesh##%d", imguiMeshID++);
	//
	// 	// Mesh Hierarchy
	// 	if (ImGui::TreeNode(imguiName))
	// 	{
	// 		auto rootNode = mesh->m_Scene->mRootNode;
	// 		MeshNodeHierarchy(mesh, rootNode);
	// 		ImGui::TreePop();
	// 	}
	// }

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	// void SceneHierarchyPanel::MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform, uint32_t level)
	// {
	// 	glm::mat4 localTransform = Mat4FromAssimpMat4(node->mTransformation);
	// 	glm::mat4 transform = parentTransform * localTransform;
	//
	// 	if (ImGui::TreeNode(node->mName.C_Str())) {
	// 		{
	// 			auto [translation, rotation, scale] = GetTransformDecomposition(transform);
	// 			ImGui::Text("World Transform");
	// 			ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
	// 			ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
	// 		}
	// 		{
	// 			auto [translation, rotation, scale] = GetTransformDecomposition(localTransform);
	// 			ImGui::Text("Local Transform");
	// 			ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
	// 			ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
	// 		}
	//
	// 		for (uint32_t i = 0; i < node->mNumChildren; i++)
	// 			MeshNodeHierarchy(mesh, node->mChildren[i], transform, level + 1);
	//
	// 		ImGui::TreePop();
	// 	}
	//
	// }

	static int s_UIContextID = 0;
	static uint32_t s_Counter = 0;
	static char s_IDBuffer[16];

	static void PushID()
	{
		ImGui::PushID(s_UIContextID++);
		s_Counter = 0;
	}

	static void PopID()
	{
		ImGui::PopID();
		s_UIContextID--;
	}

	static void BeginPropertyGrid()
	{
		PushID();
		ImGui::Columns(2);
	}

	static bool Property(const char* label, std::string& value, bool error = false)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		char buffer[256];
		strcpy(buffer, value.c_str());

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);

		if (error)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		if (ImGui::InputText(s_IDBuffer, buffer, 256))
		{
			value = buffer;
			modified = true;
		}
		if (error)
			ImGui::PopStyleColor();
		ImGui::PopItemWidth();
		ImGui::NextColumn();
		
		return modified;
	}

	static void Property(const char* label, const char* value)
	{
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		ImGui::InputText(s_IDBuffer, (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	static bool Property(const char* label, bool& value)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::Checkbox(s_IDBuffer, &value))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property(const char* label, int& value)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragInt(s_IDBuffer, &value))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property(const char* label, float& value, float delta = 0.1f)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat(s_IDBuffer, &value, delta))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property(const char* label, glm::vec2& value, float delta = 0.1f)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat2(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property(const char* label, glm::vec3& value, float delta = 0.1f)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat3(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property(const char* label, glm::vec4& value, float delta = 0.1f)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat4(s_IDBuffer, glm::value_ptr(value), delta))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static void EndPropertyGrid()
	{
		ImGui::Columns(1);
		PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		if (entity.HasComponent<T>())
		{
			bool removeComponent = false;

			auto& component = entity.GetComponent<T>();
			bool open = ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(T).hash_code()), 
				ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap 
				| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth
				, name.c_str());
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			if (ImGui::Button("+"))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::NextColumn();
				ImGui::Columns(1);
				ImGui::TreePop();
			}
			ImGui::Separator();

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		ImGui::AlignTextToFramePadding();

		auto id = entity.GetComponent<IDComponent>().ID;

		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, 256);
			memcpy(buffer, tag.c_str(), tag.length());
			if (ImGui::InputText("##Tag", buffer, 256))
			{
				tag = std::string(buffer);
			}
		}

		// ID
		ImGui::SameLine();
		ImGui::TextDisabled("%llx", id);

		ImGui::Separator();

		if (entity.HasComponent<TransformComponent>()) {
			auto& tc = entity.GetComponent<TransformComponent>();
			if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(TransformComponent).hash_code()), 
				ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth,
				"Transform")) {
				ImGui::Columns(2);
				ImGui::Text("Position");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);

				Vec3 position = tc.Position(m_TransSpace);
				if (ImGui::DragFloat3("##position", glm::value_ptr(position), 0.05f)) {
					tc.UpdatePosition(position, m_TransSpace);
				}

				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::Text("Rotation");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);

				Quat rotation = tc.Rotation(m_TransSpace);
				Vec3 eulerAngles = glm::degrees(glm::eulerAngles(rotation));
				if (ImGui::DragFloat3("##rotation", glm::value_ptr(eulerAngles), 1.f)) {
					eulerAngles = glm::radians(eulerAngles);
					tc.UpdateRotation(glm::quat(eulerAngles), m_TransSpace);
				}

				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::Text("Scale");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);

				Vec3 scale = tc.Scale(m_TransSpace);
				if (ImGui::DragFloat3("##scale", glm::value_ptr(scale), 0.05f)) {
					tc.UpdateScale(scale, m_TransSpace);
				}

				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::Columns(1);

				ImGui::TreePop();
			}
			ImGui::Separator();
		}

		DrawComponent<MeshComponent>("Mesh", entity, [](MeshComponent& mc)
		{
			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, 75);
			ImGui::SetColumnWidth(1, 200);
			ImGui::SetColumnWidth(2, 30);
			ImGui::Text("File Path");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			if (mc.Mesh)
				ImGui::InputText("##meshfilepath", (char*)mc.Mesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
			else
				ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();
			ImGui::NextColumn();
			if (ImGui::Button("...##openmesh"))
			{
				std::string file = Application::Get().OpenFile();
				if (!file.empty())
					mc.Mesh = Ref<Mesh>::Create(file);
			}
		});

		DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& cc)
		{
			ImGui::Checkbox("Primary", &cc.Primary);
			
			// Projection Type
			const char* projTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProj = projTypeStrings[(int)cc.Camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProj))
			{
				for (int type = 0; type < 2; type++)
				{
					bool is_selected = (currentProj == projTypeStrings[type]);
					if (ImGui::Selectable(projTypeStrings[type], is_selected))
					{
						currentProj = projTypeStrings[type];
						cc.Camera.SetProjectionType((SceneCamera::ProjectionType)type);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			BeginPropertyGrid();
			// Perspective parameters
			if (cc.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float verticalFOV = cc.Camera.GetPerspectiveVerticalFOV();
				if (Property("Vertical FOV", verticalFOV))
					cc.Camera.SetPerspectiveVerticalFOV(verticalFOV);

				float nearClip = cc.Camera.GetPerspectiveNearClip();
				if (Property("Near Clip", nearClip))
					cc.Camera.SetPerspectiveNearClip(nearClip);
				ImGui::SameLine();
				float farClip = cc.Camera.GetPerspectiveFarClip();
				if (Property("Far Clip", farClip))
					cc.Camera.SetPerspectiveFarClip(farClip);
			}

			// Orthographic parameters
			else if (cc.Camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = cc.Camera.GetOrthographicSize();
				if (Property("Size", orthoSize))
					cc.Camera.SetOrthographicSize(orthoSize);

				float nearClip = cc.Camera.GetOrthographicNearClip();
				if (Property("Near Clip", nearClip))
					cc.Camera.SetOrthographicNearClip(nearClip);
				ImGui::SameLine();
				float farClip = cc.Camera.GetOrthographicFarClip();
				if (Property("Far Clip", farClip))
					cc.Camera.SetOrthographicFarClip(farClip);
			}

			EndPropertyGrid();
		});

		auto DrawLightBase = [](LightComponentBase& l)
		{
			ImGui::Checkbox("Enable", &l.Enable);
			ImGui::Checkbox("Cast Shadow", &l.CastShadow);
			ImGui::ColorEdit3("Color", glm::value_ptr(l.Color));
			ImGui::DragFloat("Multiplier", &l.Multiplier, 0.1f, 0, 1000);
		};
		
		DrawComponent<DirectionLightComponent>("Direction Light", entity, [&](DirectionLightComponent& dl)
		{
			DrawLightBase(dl);
		});

		DrawComponent<PointLightComponent>("Point Light", entity, [&](PointLightComponent& dl)
		{
			DrawLightBase(dl);
			ImGui::DragFloat("Radius", &dl.Radius, 0.1, 0, 1000);
		});

		DrawComponent<SpotLightComponent>("Spot Light", entity, [&](SpotLightComponent& dl)
		{
			DrawLightBase(dl);
			ImGui::DragFloat("Radius", &dl.Radius, 0.1, 0, 1000);
			float cutOff = glm::degrees(dl.CutOff);
			if (ImGui::DragFloat("CutOff", &cutOff, 1, 0, 360)) {
				dl.CutOff = glm::radians(cutOff);
			}
		});

		auto DrawColliderBase = [](ColliderComponentBase& cc)
		{
			if (ImGui::Checkbox("Is Trigger", &cc.IsTrigger)) {
				cc.UpdateIsTrigger();
			}
			if (ImGui::DragFloat3("Center", glm::value_ptr(cc.Center), 0.05)) {
				cc.UpdateCenter();
			}
		};

		DrawComponent<BoxColliderComponent>("Box Collider", entity, [&](BoxColliderComponent& bc)
		{
			DrawColliderBase(bc);
			
			if (ImGui::DragFloat3("Size", glm::value_ptr(bc.Size), 0.05, 0, 1000)) {
				bc.UpdateSize();
			}
		});

		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [&](SphereColliderComponent& bc)
		{
			DrawColliderBase(bc);
			
			if (ImGui::DragFloat("Radius", &bc.Radius, 0.05, 0, 1000)) {
				bc.UpdateRadius();
			}
		});

		DrawComponent<RigidbodyComponent>("Rigidbody", entity, [&](RigidbodyComponent& rb)
		{
			if (ImGui::DragFloat("Mass", &rb.Mass, 0.5, 0, 10000)) {
				rb.UpdateMass();
			}
			if (ImGui::DragFloat("Drag", &rb.Drag, 0.01, 0, 1)) {
				rb.UpdateDrag();
			}
			if (ImGui::DragFloat("Angular Drag", &rb.AngularDrag, 0.01, 0, 1)) {
				rb.UpdateAngularDrag();
			}
			
			if (ImGui::Checkbox("Use Gravity", &rb.UseGravity)) {
				rb.UpdateUseGravity();
			}
			if (ImGui::Checkbox("Is Kinematic", &rb.IsKinematic)) {
				rb.UpdateIsKinematic();
			}
		});

		DrawComponent<AIControllerComponent>("AI Controller", entity, [&](AIControllerComponent& aic)
		{		
			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, 75);
			ImGui::SetColumnWidth(1, 200);
			ImGui::SetColumnWidth(2, 30);
			ImGui::Text("File Path");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			if (aic.AIController->GetBehaviorTree()) {
				ImGui::InputText("##filepath", (char*)aic.AIController->GetBehaviorTree()->GetFilepath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
			} else {
				ImGui::InputText("##filepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
			}
				
			ImGui::PopItemWidth();
			ImGui::NextColumn();
			if (ImGui::Button("...##open")) {
				std::string filepath = Application::Get().OpenFile("Behavior Tree (*.pbbt)\0*.pbbt\0");
				if (!filepath.empty()) {
					auto bt = Ref<AI::BehaviorTree>::Create(filepath);
					if (bt) {
						bt->Deserialize(bt->GetFilepath());
						aic.AIController->SetBehaviorTree(bt);
					}
				}
			}
		});

		DrawComponent<SoundSourceComponent>("Sound Source", entity, [&](SoundSourceComponent& aic)
			{
				if (ImGui::Checkbox("IsAutoPlay", &aic.SoundSource.IsAutoPlay)) {
					// 
				}
				if (ImGui::Checkbox("IsLooping", &aic.SoundSource.IsLooping)) {
					aic.SoundSource.SetLopping(aic.SoundSource.IsLooping);
				}
				if (ImGui::DragFloat("Gain", &aic.SoundSource.Gain, 0, 10)) {
					aic.SoundSource.SetGain(aic.SoundSource.Gain);
				}
				if (ImGui::DragFloat("Pitch", &aic.SoundSource.Pitch, 0, 10)) {
					aic.SoundSource.SetPitch(aic.SoundSource.Pitch);
				}
			
				ImGui::Columns(3);
				ImGui::SetColumnWidth(0, 75);
				ImGui::SetColumnWidth(1, 200);
				ImGui::SetColumnWidth(2, 30);
				ImGui::Text("File Path");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				if (!aic.Filepath.empty()) {
					ImGui::InputText("##filepath", (char*)aic.Filepath.c_str(), 256, ImGuiInputTextFlags_ReadOnly);
				}
				else {
					ImGui::InputText("##filepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
				}

				ImGui::PopItemWidth();
				ImGui::NextColumn();
				if (ImGui::Button("...##open")) {
					std::string filepath = Application::Get().OpenFile("Wav file (*.wav)\0*.wav\0");
					if (!filepath.empty()) {
						audio::s_AudioMng->BindContext(entity.GetScene()->GetAudioScene());
						aic.Filepath = filepath;
						aic.SoundSource.Load(aic.Filepath.c_str());
						audio::s_AudioMng->UnbindContext();
					}
				}
			});
		
		DrawComponent<ScriptComponent>("Script", entity, [=](ScriptComponent& sc) mutable
		{
			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, 100);
			ImGui::SetColumnWidth(1, 300);
			ImGui::SetColumnWidth(2, 40);
			ImGui::Text("File Path");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			ImGui::InputText("##meshfilepath", (char*)sc.ScriptPath.c_str(), 256, ImGuiInputTextFlags_ReadOnly);

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			if (ImGui::Button("...##openscript")) {
				std::string file = Application::Get().OpenFile();
				if (!file.empty()) {
					s_ScriptEngine->ShutdownScriptEntity(entity);
					sc.ScriptPath = file;
					s_ScriptEngine->InitScriptEntity(entity);
				}
			}
		});

	}

}
