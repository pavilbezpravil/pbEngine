#include "pch.h"
#include "SceneRenderer.h"

#include "GraphicsCommon.h"
#include "CommandContext.h"
#include "ColorBuffer.h"
#include "Renderer.h"

namespace pbe {

	void SceneRenderer::Init()
	{
		vs = Shader::Get(L"base", NULL, "mainVS", ShaderType::Vertex);
		ps = Shader::Get(L"base", NULL, "mainPS", ShaderType::Pixel);

		InitBaseRootSignature();
	}

	void SceneRenderer::BeginScene(const Ref<Scene>& scene, const CameraInfo& cameraInfo)
	{
		HZ_CORE_ASSERT(!_scene);
		_scene = scene;
		_cameraInfo = cameraInfo;
	}

	void SceneRenderer::EndScene()
	{
		HZ_CORE_ASSERT(_scene);
		FlushDrawList();
		_scene = nullptr;
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
	{
		DrawCommand cmd = {mesh, transform};
		_drawList.push_back(cmd);
	}

	void SceneRenderer::FlushDrawList()
	{
		GraphicsContext& context = GraphicsContext::Begin(L"Scene Renderer");

		auto& rt = Renderer::Get().GetFullScreenColor();
		auto& depth = Renderer::Get().GetFullScreenDepth();

		context.ClearColor(rt);
		context.ClearDepth(depth);
		
		context.SetRenderTarget(rt, depth);
		context.SetViewportAndScissor(0, 0, rt->GetWidth(), rt->GetHeight());

		context.SetRootSignature(BaseRootSignature);
		context.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context.SetDepthStencilState(Graphics::DepthStateReadWrite);
		
		context.SetVertexShader(vs);
		context.SetPixelShader(ps);

		FVF fvf = FVF_POS | FVF_NORMAL;
		auto inputLayout = fvfGetInputLayout(fvf);
		InitBaseRootSignature();
		context.SetInputLayout((UINT)inputLayout.size(), inputLayout.data());

		struct cbPass {
			Mat4 gMVP;
		};

		cbPass pass;
		pass.gMVP = _cameraInfo.viewProj;
		context.SetDynamicConstantBufferView(0, sizeof(cbPass), &pass);

		for (auto& drawCmd : _drawList) {
			// todo: transform
			auto& mesh = drawCmd.mesh;
			for (auto& submesh : mesh->GetSubmeshes()) {
				struct cbModel {
					Mat4 gTransform;
					Mat4 gNormalTransform;
				};

				cbModel model;
				model.gTransform = drawCmd.transform * submesh.Transform;
				model.gNormalTransform = glm::transpose(glm::inverse(model.gTransform));
				context.SetDynamicConstantBufferView(1, sizeof(model), &model);

				context.SetVertexBuffer(0, mesh->GetVertexBuffer()->VertexBufferView(submesh.BaseVertex));
				context.SetIndexBuffer(mesh->GetIndexBuffer()->IndexBufferView(submesh.BaseIndex));
				context.DrawIndexed(submesh.IndexCount, 0, 0);
			}
		}

		context.Finish();

		_drawList = {};
	}

}
