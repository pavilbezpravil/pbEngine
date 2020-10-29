#include "pch.h"
#include "Renderer.h"

#include "Shader.h"

#include "RendererAPI.h"
#include "SceneRenderer.h"
#include "Renderer2D.h"
#include "ColorBuffer.h"
#include "GraphicsCore.h"
#include "pbe/Core/Application.h"

// dx12
extern DXGI_FORMAT SwapChainFormat;

namespace pbe {

	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::DX12;

	Ref<ColorBuffer> g_finalRT;
	uint g_width = 0;
	uint g_height = 0;
	DXGI_FORMAT g_finalRTFormat;

	struct RendererData
	{
		RenderCommandQueue m_CommandQueue;

		Ref<VertexBuffer> m_FullscreenQuadVertexBuffer;
		Ref<IndexBuffer> m_FullscreenQuadIndexBuffer;
		Ref<Pipeline> m_FullscreenQuadPipeline;
	};

	static RendererData s_Data;
	
	void Renderer::Init()
	{
		auto& window = Application::Get().GetWindow();
		Graphics::Initialize(*(HWND*)window.GetNativeWindowHandler(), window.GetWidth(), window.GetHeight());

		g_finalRTFormat = SwapChainFormat;
		g_finalRT = Ref<ColorBuffer>(new ColorBuffer);

		Renderer::Resize(window.GetWidth(), window.GetHeight());

		// Renderer::Submit([](){ RendererAPI::Init(); });

		// SceneRenderer::Init();

		// Renderer2D::Init();
	}

	void Renderer::Shutdown() {
		Graphics::Shutdown();
	}

	void Renderer::Resize(uint width, uint height) {
		if (g_width == width || g_height == height) {
			return;
		}
		g_width = width;
		g_height = height;
		g_finalRT->Create(L"Final RT", g_width, g_height, 1, g_finalRTFormat);
	}

	Ref<ColorBuffer>& Renderer::GetFinalRT() {
		return g_finalRT;
	}

	void Renderer::Clear()
	{
		Renderer::Submit([](){
			RendererAPI::Clear(0.0f, 0.0f, 0.0f, 1.0f);
		});
	}

	void Renderer::Clear(float r, float g, float b, float a)
	{
		Renderer::Submit([=]() {
			RendererAPI::Clear(r, g, b, a);
		});
	}

	void Renderer::ClearMagenta()
	{
		Clear(1, 0, 1);
	}

	void Renderer::SetClearColor(float r, float g, float b, float a)
	{
	}

	void Renderer::DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawIndexed(count, type, depthTest);
		});
	}

	void Renderer::SetLineThickness(float thickness)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetLineThickness(thickness);
		});
	}

	void Renderer::WaitAndRender()
	{
		s_Data.m_CommandQueue.Execute();
	}

	void Renderer::SubmitQuad(Ref<MaterialInstance> material, const glm::mat4& transform)
	{
		bool depthTest = true;
		if (material)
		{
			material->Bind();
			depthTest = material->GetFlag(MaterialFlag::DepthTest);

			auto shader = material->GetShader();
			// shader->SetMat4("u_Transform", transform);
		}

		s_Data.m_FullscreenQuadVertexBuffer->Bind();
		s_Data.m_FullscreenQuadPipeline->Bind();
		s_Data.m_FullscreenQuadIndexBuffer->Bind();
		Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
	}

	void Renderer::SubmitFullscreenQuad(Ref<MaterialInstance> material)
	{
		bool depthTest = true;
		if (material)
		{
			material->Bind();
			depthTest = material->GetFlag(MaterialFlag::DepthTest);
		}

		s_Data.m_FullscreenQuadVertexBuffer->Bind();
		s_Data.m_FullscreenQuadPipeline->Bind();
		s_Data.m_FullscreenQuadIndexBuffer->Bind();
		Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
	}

	void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
	{
		// auto material = overrideMaterial ? overrideMaterial : mesh->GetMaterialInstance();
		// auto shader = material->GetShader();
		// TODO: Sort this out
		mesh->m_VertexBuffer->Bind();
		mesh->m_Pipeline->Bind();
		mesh->m_IndexBuffer->Bind();

		auto& materials = mesh->GetMaterials();
		for (Submesh& submesh : mesh->m_Submeshes)
		{
			// Material
			auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
			auto shader = material->GetShader();
			material->Bind();

			if (mesh->m_IsAnimated)
			{
				for (size_t i = 0; i < mesh->m_BoneTransforms.size(); i++)
				{
					std::string uniformName = std::string("u_BoneTransforms[") + std::to_string(i) + std::string("]");
					// mesh->m_MeshShader->SetMat4(uniformName, mesh->m_BoneTransforms[i]);
				}
			}
			// shader->SetMat4("u_Transform", transform * submesh.Transform);

			Renderer::Submit([submesh, material]() {
				// if (material->GetFlag(MaterialFlag::DepthTest))	
				// 	glEnable(GL_DEPTH_TEST);
				// else
				// 	glDisable(GL_DEPTH_TEST);
				//
				// glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
			});
		}
	}

	void Renderer::DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color)
	{
		for (Submesh& submesh : mesh->m_Submeshes)
		{
			auto& aabb = submesh.BoundingBox;
			auto aabbTransform = transform * submesh.Transform;
			DrawAABB(aabb, aabbTransform);
		}
	}

	void Renderer::DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color /*= glm::vec4(1.0f)*/)
	{
		glm::vec4 min = { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f };
		glm::vec4 max = { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f };

		glm::vec4 corners[8] =
		{
			transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f },

			transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f }
		};

		for (uint32_t i = 0; i < 4; i++)
			Renderer2D::DrawLine(corners[i], corners[(i + 1) % 4], color);

		for (uint32_t i = 0; i < 4; i++)
			Renderer2D::DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

		for (uint32_t i = 0; i < 4; i++)
			Renderer2D::DrawLine(corners[i], corners[i + 4], color);
	}

	RenderCommandQueue& Renderer::GetRenderCommandQueue()
	{
		return s_Data.m_CommandQueue;
	}

}