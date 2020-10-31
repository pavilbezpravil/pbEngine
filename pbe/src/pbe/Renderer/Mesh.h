#pragma once

#include <vector>
#include <glm/glm.hpp>


#include "Texture.h"
#include "pbe/Core/Timestep.h"

#include "pbe/Renderer/IndexBuffer.h"
#include "pbe/Renderer/VertexBuffer.h"
#include "pbe/Renderer/Shader.h"

#include "pbe/Core/Math/AABB.h"
#include "pbe/Geom/GeomBuffer.h"

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp {
	class Importer;
}

namespace pbe {

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;

		uint32_t IDs[4] = { 0, 0,0, 0 };
		float Weights[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t BoneID, float Weight)
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (Weights[i] == 0.0)
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}

			// TODO: Keep top weights
			HZ_CORE_WARN("Vertex has more than four bones/weights affecting it, extra data will be discarded (BoneID={0}, Weight={1})", BoneID, Weight);
		}
	};

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	struct VertexBoneData
	{
		uint32_t IDs[4];
		float Weights[4];

		VertexBoneData()
		{
			memset(IDs, 0, sizeof(IDs));
			memset(Weights, 0, sizeof(Weights));
		};

		void AddBoneData(uint32_t BoneID, float Weight)
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (Weights[i] == 0.0)
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}
			
			// should never get here - more bones than we have space for
			HZ_CORE_ASSERT(false, "Too many bones!");
		}
	};

	class Submesh
	{
	public:
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex;
		uint32_t IndexCount;

		glm::mat4 Transform;
		AABB BoundingBox;

		std::string NodeName, MeshName;
	};

	class Mesh : public RefCounted
	{
	public:
		Mesh(const std::string& filename);
		~Mesh();

		void OnUpdate(Timestep ts);
		void DumpVertexBuffer();

		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

		Ref<Shader> GetMeshShader() { return m_MeshShader; }
		const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }
		const std::string& GetFilePath() const { return m_FilePath; }
	private:
		void BoneTransform(float time);
		void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);

		const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		uint32_t FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint32_t FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint32_t FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
		glm::vec3 InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::quat InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim);
	private:
		std::vector<Submesh> m_Submeshes;
		
		std::unique_ptr<Assimp::Importer> m_Importer;

		glm::mat4 m_InverseTransform;

		uint32_t m_BoneCount = 0;
		std::vector<BoneInfo> m_BoneInfo;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		GeomBuffer m_GeomData;
		std::unordered_map<std::string, uint32_t> m_BoneMapping;
		std::vector<glm::mat4> m_BoneTransforms;
		const aiScene* m_Scene;

		// Materials
		Ref<Shader> m_MeshShader;
		std::vector<Ref<Texture2D>> m_Textures;
		std::vector<Ref<Texture2D>> m_NormalMaps;

		// Animation
		bool m_IsAnimated = false;
		float m_AnimationTime = 0.0f;
		float m_WorldTime = 0.0f;
		float m_TimeMultiplier = 1.0f;
		bool m_AnimationPlaying = true;

		std::string m_FilePath;

		friend class Renderer;
		friend class SceneHierarchyPanel;
	};
}
